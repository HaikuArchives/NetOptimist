
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <storage/Directory.h>
#ifndef __BEOS__
#include <libgen.h>
#endif
#include <Message.h>
#include "platform.h"
#include "Url.h"
#include "UrlQuery.h"
#include "Resource.h"
#include "ResourceGetter.h"
#include "Http.h"
#include "traces.h"
#include "Pref.h"
#include "StrPlus.h"

int httpGetter(Url *url, Resource** rsc);
int fileGetter(Url *url, Resource **rsc);

const Protocol protocol_list[] = {
	{ HttpId, "http:", true, 80, httpGetter },
	{ FileId, "file:", false, 0, fileGetter },
	{ HttpsId, "https:", true, 443, NULL },
	{ MailToId, "mailto:", false, 0, NULL },
	{ FtpId, "ftp:", true, 21, NULL },
	{ BoggusId, NULL, false, 0, NULL }
};

static char *my_dirname(register char *pathname)
{
	register char  *last;
	/* go to end of path */
	for(last=pathname; *last; last++);
	// back to trailing /
	if (last>pathname) last--;
	while(last>pathname && *last!='/') last--;
	// skip multiple trailing /
	while(last>pathname && *(last-1)=='/') last--;
	*last = 0;
	return(pathname);
}

static const char* my_basename(const char *path) {
	// Obviously this is a poor replacement for basename but enough
	// for our needs
	const char *last = path;
	while (*path) {
		if (*path=='/') last=path;
		path++;
	}
	return last+1;
}

#ifdef __BEOS__
static const char cacheLog[] = "/boot/home/config/settings/NetOptimist/NetCache/CacheLog";
#else
static const char cacheLog[] = "CacheLog";
#endif

struct Cache::cacheLine {
	cacheLine *next;
	StrRef url;
	int int1; // XXX Currently unused
	enum {
		TYPE_NONE, TYPE_PENDING, TYPE_FILEREF, TYPE_MEMORY
	} m_type;
	Resource *resource;
	bool IsValid();
	void Remove();
};

bool Cache::cacheLine::IsValid() {
	if (!resource) return false;
	time_t now = time(0);
	int in_cache = now-resource->m_date;
	int age_when_downloaded = resource->m_date-resource->m_modified;
	if (in_cache>60*60 && in_cache > age_when_downloaded) {
		// This resource has been to long in our cache
		return false;
	}
	if ((resource->m_expires>0 && resource->m_expires<now)
			 || now-resource->m_date>5*24*60*60) {
		return in_cache<10; // This is the minimum amount of time of validity
	}
	return true;
}

void Cache::cacheLine::Remove() {
	switch(m_type) {
		case TYPE_NONE :
		case TYPE_PENDING :
			// Nothing
			break;
		case TYPE_FILEREF :
		{
			struct stat buf;
			if (stat (resource->CachedFile(), &buf) >= 0
					&& (buf.st_mode & S_IFREG) == S_IFREG) {
				if (remove(resource->CachedFile())==0) {
					fprintf(stdout, "Cache: removed %s\n", resource->CachedFile());
				} else {
					fprintf(stderr, "Cache: could not remove %s : %s\n", resource->CachedFile(), strerror(errno));
				}
			}
			delete resource;
			resource = NULL;
			m_type = TYPE_NONE;
			break;
		}
		case TYPE_MEMORY:
		{
			delete resource;
			resource = NULL;
			m_type = TYPE_NONE;
			break;
		}
	}
}

Cache::Cache ()
{
}

void Cache::Init()
{
	char line[1000];
	char url[1000];

	::create_directory ("/boot/home/config/settings/NetOptimist/NetCache", 0777);

	trace(DEBUG_URL) {
		/* This is for testing purpose only */
		char *dirs_in[] = { "/", "/truc", "/truc/", "/truc/bidule", "/truc/bidule/", NULL };
		char *dirs_out[] ={ "",  "",	  "/truc",  "/truc",	    "/truc/bidule",  NULL };
		for (int i = 0; dirs_in[i]; i++) {
			char *s = strdup(dirs_in[i]);
			my_dirname(s);
			if (strcmp(s,dirs_out[i])) {
				fprintf(stderr, "ERROR : dirname(%s)=%s doesn't match expected result %s\n", dirs_in[i], s, dirs_out[i]);
			}
			free(s);
		}
	}

	nbLine = 0;
	cacheLines = NULL;
	FILE *cacheIndex = fopen (cacheLog, "r");
	if (cacheIndex) {
		cacheLine *l = NULL;
		cacheLine *last = NULL;
		while (fgets (line, sizeof(line)-1, cacheIndex) > 0) {
			l = new cacheLine;
			nbLine++;

			char file[100];
			char mimetype[50];
			time_t date1, date2, date3;
			size_t size;
			sscanf (line, "%ld%ld%ld%s%s%s%d%ld", &date1, &date2, &date3,
				file, url, mimetype, &l->int1, &size);
			l->m_type = cacheLine::TYPE_FILEREF;
			l->url.SetToDup (url);

			trace(DEBUG_CACHE) {
				printf("Creating resource %s url %s\n", file, url);
			}
			l->resource = new CacheResource(file, size);
			l->resource->m_date = date1;
			l->resource->m_expires = date2;
			l->resource->m_modified = date3;
			l->resource->SetMimeType (mimetype);

			if (l->resource->m_expires>0) {
				trace(DEBUG_CACHE) {
					char datestr[30];	// from ctime_r man page : "at least 26"
					time_t t = l->resource->m_expires;
					ctime_r(&t, datestr);
					char * p = datestr;
					while (*(p+1)) p++; if (*p=='\n') *p = '\0';
					fprintf(stderr, "Url %s expires at %s - in %ld seconds\n", l->url.Str(), datestr, t-time(NULL));
				}
			} 
			if (!l->IsValid()) {
				l->Remove();
				delete l;
			} else {
				l->next = NULL;
				// Add the element in the cache list
				if (cacheLines) {
					last->next = l;
					last = l;
				}
				else {
					cacheLines = last = l;
				}
			}
		}
		fclose (cacheIndex);
	}
	m_getter = NULL;
}


void Cache::SetResourceGetter (ResourceGetter * getter) {
	m_getter = getter;
}

void Cache::ClearAll() {
	cacheLine *l = cacheLines;
	cacheLine *p;
	while (l) {
		if (l->url.Str()) {
			l->Remove();
			delete l->resource;
		}
		p = l;
		l = l->next;
		delete p;
	}
	cacheLines = NULL;
}

Cache::~Cache () {
	cacheLine *l = cacheLines;
	cacheLine *p;
	int nbRes = 0;

	FILE *cacheOutput = fopen (cacheLog, "w");

	unsigned int magic_id;
	magic_id = (int)time(NULL);
	
	while (l) {
		if (l->url.Str()) {
			if (cacheOutput) {
				switch(l->m_type) {
				case cacheLine::TYPE_FILEREF:
				{
					const char *filename;
					if (strncmp(l->resource->CachedFile(), Pref::Default.CacheLocation(), strlen(Pref::Default.CacheLocation()))==0)
						filename = my_basename(l->resource->CachedFile());
					else
						filename = l->resource->CachedFile();

					fprintf (cacheOutput, "%ld %ld %ld %s %s %s %d %ld\n", l->resource->m_date, l->resource->m_expires,
						l->resource->m_modified, filename, l->url.Str(), l->resource->MimeType (),
						l->int1, l->resource->Size());
				}
				break;
				case cacheLine::TYPE_MEMORY:
				{
					if (l->resource && l->resource->m_cacheOnDisk && l->resource->Size()>0) { // Don't save an empty file
						const char *mimetype = l->resource->MimeType();
						while (*mimetype && *mimetype != '/')
							mimetype++;
						if (*mimetype == '/') mimetype++;

						char absfname[500];
						char *fname;
						strcpy(absfname, Pref::Default.CacheLocation());
						fname = absfname + strlen(absfname);
						if (fname>absfname && fname[-1]!='/') { *fname = '/'; fname++; }
						if (!strnull(mimetype)) {
							sprintf(fname, "Cache%u-%d.%s", magic_id, nbRes, mimetype);
						} else {
							sprintf(fname, "Cache%u-%d", magic_id, nbRes);
						}
						const char * url = l->resource->m_location ?
							l->resource->m_location : l->url.Str();
						if (l->resource->SaveToFile(absfname)) {
							fprintf (cacheOutput, "%ld %ld %ld %s %s %s %d %ld\n",
								l->resource->m_date, l->resource->m_expires,
								l->resource->m_modified, fname, url,
								l->resource->MimeType(), l->int1,
								l->resource->Size());
						}
						nbRes++;
					}
				}
				break;
				case cacheLine::TYPE_PENDING:
					fprintf(stderr, "Cache : url %s still pending\n", l->url.Str());
				break;
				case cacheLine::TYPE_NONE:
					fprintf(stderr, "Cache : url %s has expired\n", l->url.Str());
				break;
				}
			}
			l->url.Free();
			delete l->resource;
		}
		p = l;
		l = l->next;
		delete p;
	}
	if (cacheOutput)
		fclose (cacheOutput);
}


Resource *Cache::Find (Url * url) {
	Resource *rsc = NULL;
	struct stat buf;

	StrRef ref;
	url->ToStrRef(&ref);
	const char *urlAbsolute = ref.Str();

	cacheLine *l = cacheLines;
	while (l && !rsc) {
		if (!l->resource) {
			trace (DEBUG_CACHE)
				fprintf (stderr, "Cache::Find no resource for this entry\n");
		} else {
			if (strcmp (l->url.Str(), urlAbsolute)==0) {
				const char *file;
				trace (DEBUG_CACHE)
					fprintf (stderr, "Cache::Find FOUND url %s\n", urlAbsolute);
				if (l->IsValid()) {
					rsc = l->resource;
					switch (l->m_type) {
					    case cacheLine::TYPE_FILEREF:
						file = rsc->CachedFile();
						if (stat (file, &buf) >= 0 && (buf.st_mode & S_IFREG) == S_IFREG) {
							if (rsc->Size() == 0)
								rsc->SetSize(buf.st_size);
							trace (DEBUG_CACHE)
								fprintf (stderr, "Cache: Url %s found in file %s\n", urlAbsolute, file);
							trace (DEBUG_CACHE) {
							    if (rsc->m_expires>0)
								fprintf (stderr, "Cache: Url %s expires in %d\n", urlAbsolute, (int)(rsc->m_expires-time(0)));
							}
						} else {
							trace (DEBUG_CACHE) {
								printf("Cache:Find cannot stat file %s\n", file);
							}
							l->Remove();
							rsc = NULL;
						}
						break;
					    case cacheLine::TYPE_MEMORY:
						/* Found url in memory */
						break;
					    case cacheLine::TYPE_NONE:
					    case cacheLine::TYPE_PENDING:
						rsc = NULL;
						/* Not ready */
						break;
					}
				} else {
					trace (DEBUG_CACHE)
						fprintf (stderr, "Cache: Url %s has expired\n", urlAbsolute);
				}
			}
		}
		l = l->next;
	}

	return rsc;
}


Cache::cacheLine *Cache::FindCacheLine(const char *url) {
	cacheLine *l = cacheLines;
	while (l) {
		if (l->url.Str() && !strcmp(l->url.Str(), url)) {
			trace (DEBUG_CACHE)
				printf("FindCacheLine : %s already has a cache entry\n", url);
			break;
		}
		l = l->next;
	}
	return l;
}

bool Cache::AddResource(const char *url, Resource *rsc) {
	//Add this resource in the cache

	bool created = false;
	bool empty = false;

	cacheLine *l = NULL;
	l = FindCacheLine(url);
	if (!l) {
		// If no matching cache line found, create a new one
		nbLine++;
		created = true;
		l = new cacheLine;
		l->int1 = 0;
		l->next = NULL;
		l->url.SetToDup (url);
		l->resource = NULL;
		l->m_type = cacheLine::TYPE_PENDING;
	}
	if (rsc) {
		l->m_type = cacheLine::TYPE_MEMORY;
		if (l->resource) fprintf(stderr, "ERROR : overriding existing data\n");
		l->resource = rsc;
	} else {
		empty = created || l->m_type == cacheLine::TYPE_NONE;
	}
	if (created) {
		// Chain the newly created cache entry
		l->next = cacheLines;
		cacheLines = l;
	}
	// Do we need to start a download ?
	return empty;
}

Resource *Cache::Retrieve (Url * url, bool async, bool reformat) {
	Resource *rsc = NULL;

	StrRef ref;
	url->ToStrRef(&ref);
	const char *urlAbsolute = ref.Str();

	rsc = Find (url);

	if (!rsc) {
		if (!async) {
			const Protocol *proto = url->Protocol();
				// If the protocol has host in url, we need to be online to connect
				// Otherwise, protocol getter is allowed
			if (!proto->hasHost || Pref::Default.Online()) {
				if (proto->protocolGetter) {
					trace (DEBUG_FILE)
						printf("Cache->%s for url %s ...\n", proto->name, urlAbsolute);
					int status = proto->protocolGetter(url, &rsc);
					if (status) {
						fprintf(stderr, "%s error when downloading : %s\n", proto->name, urlAbsolute);
					}
				}
				if (rsc) {
					//Add this resource in the cache
					AddResource(urlAbsolute, rsc);
					if (!strnull(rsc->m_location)) {
						StrRef r;
						r.SetToDup(rsc->m_location);
						url->SetDisplayName(&r);
					}
				}
			}
		} else {
			if (AddResource(urlAbsolute, NULL)) {
				if (Pref::Default.Online()) {
					trace (DEBUG_CACHE)
						printf("Cache::Retreive queueing url %s\n", urlAbsolute);
					BMessage msg (GET_IMAGE);
					msg.AddString ("url", urlAbsolute);
					msg.AddBool ("reformat", reformat);
					m_getter->PostMessage (&msg);
				}
			} else {
				trace (DEBUG_CACHE)
					printf("Cache::Retreive: Not http'ing %s -- download pending...\n", urlAbsolute);
			}
		}
	}

	if (! rsc && !async) {
		url->SetError();
	}
	return rsc;
}

Cache Cache::cache;


Url::Url (const char *urlName, const Url *base, bool async, bool reformat) {
	UrlQuery query;
	query.SetUrl(urlName);
	_SetTo(&query, base, async, reformat);
}

Url::Url (const UrlQuery *urlQuery, const Url *base, bool async, bool reformat) {
	_SetTo(urlQuery, base, async, reformat);
}

void Url::_SetTo(const UrlQuery *urlQuery, const Url *base, bool async, bool reformat) {
	const char *urlName = urlQuery->Url();
	m_host[0] = '\0';
	m_port = 0;
	m_protocol = NULL;
	m_resource = NULL;
	m_error = false;

	trace (DEBUG_URL) {
		StrRef ref;
		if (base)
			base->ToStrRef(&ref);
		else
			ref.SetToConst("(none)");
		fprintf(stderr, "new URL for %s (base=%s)\n", urlName, ref.Str());
	}
	/* We are passed in a sting representing an url. Remember that common url forms include :
		- http://www.somewhere.mars/life/water.html	(most common form)
		- //www.somewhere.mars/life/water.html		(implicit protocol from base url)
		- file:///etc/passwd				("local protocols" doesn't specify hostname, but...)
		- file:/etc/passwd or file://etc/passwd		(these ones can be used too)
	*/

	// Extracting protocol
	for (const struct Protocol *proto = protocol_list; proto->name!=NULL; proto++) {
		int len;
		if ((len=strprefix(urlName, proto->name))>0) {
			m_protocol = proto;
			urlName += len;
			trace(DEBUG_URL) printf("URL: found protocol %s\n", proto->name);
			break;
		}
	}
	if (m_protocol == NULL && base != NULL) {
		m_protocol = base->m_protocol;
	}
	if (m_protocol==NULL) {
		/* By default, we set file:// protocol */
		for (const struct Protocol *proto = protocol_list; proto->name!=NULL; proto++) {
			if (!strcmp(proto->name, "file:")) {
				m_protocol = proto;
				break;
			}
		}
	}

	if (strprefix(urlName, "//")>0) {
		// skip leading //
		urlName += 2;

		if (m_protocol && m_protocol->hasHost) {
			//Extract hostname
			int nextsep = strcspn (urlName, "/:"); //this will point to / : or \0

			strncpy (m_host, urlName, nextsep);
			m_host[nextsep] = '\0';
			urlName += nextsep;

			trace(DEBUG_URL) printf("URL: Decoding hostname %s\n", m_host);

			//Extract port
			if (urlName[0] == ':') {
				urlName++;
				m_port = atoi (urlName);		//Reads port number
				urlName = strchr (urlName, '/');	//find the leading /
				trace(DEBUG_URL) printf("URL: Decoding port %d\n", m_port);
			}
			if (m_port == 0) {
				//no port specified or could not read port correctly
				m_port = m_protocol->defaultPort;
			}
		}
	}
	char *ptr = m_file;
	while (*urlName && *urlName!='#')
		*ptr++ = *urlName++;
	*ptr = '\0';

	if (*urlName=='#') {
		urlName++;
		strcpy(m_target, urlName);
	}

	if (strnull(m_file)) {
		strcpy (m_file, "/"); //this is the simpliest file part in URL
	}

	m_method = urlQuery->m_method;
	m_nbParameters = urlQuery->m_nbFields;
	if (m_nbParameters>0)
		m_parameters = urlQuery->m_fields;
	else
		m_parameters = NULL;

	trace(DEBUG_URL) printf("URL: Decoding file : %s\n", m_file);

	// If a base url was passed, compute absolute url
	if (base)
		ToAbsolute (base);

	// Construct a string representing url (in absolute form)
	int len = strlen(m_file) + strlen(m_protocol->name);
	if (!strnull(m_host))
		len += strlen(m_host);
	char *str = new char[len+10]; // reverve space for port and :// etc...

	const char *host = (!strnull(m_host)) ? m_host : "";

	if (m_port == m_protocol->defaultPort)
		sprintf(str, "%s//%s%s", m_protocol->name, host, m_file);
	else
		sprintf(str, "%s//%s:%d%s", m_protocol->name, m_host, m_port, m_file);

	m_urlStrRef.SetToString(str, false);

// XXX he c'est quoi ca ? on download avant d'en avoir besoin ?
	m_resource = Cache::cache.Retrieve (this, async, reformat);
		
	if (!m_resource && !async) {
		fprintf (stderr, "Url %s not found\n", m_urlStrRef.Str());
	}
}

void Url::CopyFrom(const Url* url) {
	this->m_protocol = url->m_protocol;
	this->m_port = url->m_port;
	strcpy(this->m_host, url->m_host);
	strcpy(this->m_file, url->m_file);
	strcpy(this->m_target, url->m_target);
	this->m_resource = NULL;
	this->m_error = false;
}

void Url::SetError() {
	m_error = true;
}

Url::~Url () {
}


void Url::ToAbsolute (const Url * basUrl) {

	if (m_protocol==NULL) {
		m_protocol = basUrl->m_protocol;
	}
	if (strnull(m_host)) {
		strcpy (m_host, basUrl->m_host);
		m_port = basUrl->m_port;
	}
	if (m_file[0] != '/') {
		/*
		 * Here is the difficult part : we must compute the path from the
		 * base url and add the relative part
		 */

		char * new_file;
		new_file = (char*)malloc(strlen(m_file) + strlen(basUrl->m_file)+1);
		strcpy(new_file, basUrl->m_file);

		trace (DEBUG_URL) fprintf(stderr, "URL DIRNAME avant %s\n", new_file);
		my_dirname(new_file);
		trace (DEBUG_URL) fprintf(stderr, "URL DIRNAME apres %s\n", new_file);
		
		// XXX this doesn't handle url of type .././.././../xxx.html
		while (strncmp (m_file, "./", 2) == 0) {
			strcpy (m_file, m_file + 2);
		}
		while (strncmp (m_file, "../", 3) == 0) {
			trace (DEBUG_URL) fprintf(stderr, "URL DIRNAME(..) avant %s + %s\n", new_file, m_file);
			strcpy (m_file, m_file + 3);
			my_dirname(new_file);
			trace (DEBUG_URL) fprintf(stderr, "URL DIRNAME(..) apres %s + %s\n", new_file, m_file);
		}
		strcat(new_file, "/");
		strcat(new_file, m_file);
		strcpy(m_file, new_file);
		free(new_file);
	}
}


int Url::FilePart (char *str) const {
	return sprintf (str, "%s", m_file);
}


int Url::HostAndPortStr (char *str) const {
	int nb;
	if (m_port == m_protocol->defaultPort)
		nb = sprintf (str, "%s", m_host);
	else
		nb = sprintf (str, "%s:%u", m_host, m_port);
	return nb;
}

const Protocol *Url::Protocol () const {
	return m_protocol;
}

const char *Url::Host () const {
	return m_host;
}

const char *Url::File () const {
	return m_file;
}

unsigned Url::Port () const {
	return m_port;
}

int Url::NbParams() const {
	return m_nbParameters;
}

const char * Url::ParamAt(int i) const {
	return m_parameters[i];
}

ProtocolMethod Url::Method() const {
	return m_method;
}

void Url::ToStrRef(StrRef *strRef) const {
	strRef->SetToRef(&m_urlStrRef);
}

int Url::ToString (char *str) const {
	return sprintf(str, "%s", m_urlStrRef.Str());
}


Resource *Url::GetIfAvail() {
	if (!m_resource)
		m_resource = Cache::cache.Find (this);
	return m_resource;
}

Resource *Url::GetDataNow() {
	if (!m_resource && !m_error)
		m_resource = Cache::cache.Retrieve (this, false, false);
	return m_resource;
}

int httpGetter(Url *url, Resource** rsc) {
	*rsc = NULL;

	ServerConnection *server =
		ConnectionMgr::Default.GetConnection (url);

	if (server) {
		int protocol_result;
		server->OpenConnection (url);
		server->PrepareHeader (url);
		server->SendHeader (url);
		server->ReadHeader (&protocol_result);

		if (protocol_result != 204) {
			// XXX Code 204 => no body but why ?
			*rsc = server->GetData ();
		}
		StrRef reason;
		if (!server->Status(&reason)) {
			if (!reason.IsFree()) {
				fprintf(stderr, "Reason : %s\n", reason.Str());
			} else {
				fprintf(stderr, "Unknown http reason\n");
			}
		}
		ConnectionMgr::Default.ReleaseConnection (server);
	} else {
		fprintf (stderr, "Could not get connection to Server\n");
	}
	if (*rsc) return 0;
	else	  return -1;
}

int fileGetter(Url *url, Resource **rsc) {
	* rsc = new FileResource(url->File());
	if (*rsc) return 0;
	else	  return -1;
}
