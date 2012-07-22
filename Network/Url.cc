
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

int httpGetter(Url *url, Resource** rsc, time_t);
int fileGetter(Url *url, Resource **rsc, time_t);

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

#if defined __BEOS__ || __HAIKU__
static const char cacheLog[] = "/boot/home/config/settings/NetOptimist/NetCache/CacheLog";
#else
static const char cacheLog[] = "CacheLog";
#endif

struct Cache::cacheLine {
	cacheLine *next;
	StrRef url;
	int int1; // XXX Currently unused
	enum {
		TYPE_NONE, TYPE_FILEREF, TYPE_MEMORY
	} m_type;
	enum {
		STATE_PENDING, STATE_OK, STATE_VALIDATING, STATE_FAILED
	} m_state;
	Resource *resource;
	bool IsValid();
	bool NeedValidate();
	void Remove();
};

bool Cache::cacheLine::IsValid() {
	if (!resource) return false;
	if (!Pref::Default.Online()) return true; 
	time_t now = time(0);
	int in_cache = now-resource->m_date;
	int age_when_downloaded = resource->m_modified>0 ? resource->m_date-resource->m_modified : 0;
	if (in_cache>60*60 && in_cache > age_when_downloaded) {
		// This resource has been to long in our cache
		if (ISTRACE(DEBUG_CACHE))
			printf("IsValid: in cache since %ds - age when downloaded %ds\n", in_cache, age_when_downloaded);
		return false;
	}
	return ((resource->m_expires==0 || resource->m_expires>=now)
			 && in_cache<5*24*60*60);
}

bool Cache::cacheLine::NeedValidate() {
	if (!Pref::Default.Online()) return false; 
	time_t now = time(0);
	int in_cache = now-resource->m_date;
	int age_when_downloaded = resource->m_modified>0 ? resource->m_date-resource->m_modified : 0;
	if (in_cache>60*60*8 || (in_cache>60 && in_cache > age_when_downloaded/8)) {
		// This resource has been to long in our cache
		if (ISTRACE(DEBUG_CACHE))
			printf("NeedValidate: in cache since %ds - age when downloaded %ds\n", in_cache, age_when_downloaded);
		return true;
	}
	return (resource->m_expires>0 && resource->m_expires<=now);
}

void Cache::cacheLine::Remove() {
	switch(m_type) {
		case TYPE_NONE :
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

void Cache::ClearByProtocolAndHost(const char *protocolAndHost, const char*mimetype) {
	cacheLine *l = cacheLines;
	cacheLine *prev = NULL;
	while (l) {
		cacheLine *t;
		t = l;
		l = l->next;
		if (strprefix(t->url.Str(), protocolAndHost) != 0 && (mimetype==NULL || strcmp(t->resource->MimeType(),mimetype)==0)) {
			if (prev)
				prev->next = l;
			else
				cacheLines = l;
			t->Remove();
			delete t->resource;
		} else {
			prev = t;
		}
	}
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
						} else {
							fprintf (stderr, "Could not save resource for %s\n", url);
						}
						nbRes++;
					}
				}
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

	StrRef ref;
	url->ToStrRef(&ref);
	const char *urlAbsolute = ref.Str();

	cacheLine *l = FindCacheLine(urlAbsolute);
	while (l && !rsc) {
		if (!l->resource) {
			trace (DEBUG_CACHE)
				fprintf (stderr, "Cache::Find no resource for this entry\n");
		} else {
			const char *file;
			trace (DEBUG_CACHE)
				fprintf (stderr, "Cache::Find FOUND url %s\n", urlAbsolute);
			if (l->m_state == cacheLine::STATE_OK) {
				if (l->IsValid()) {
					struct stat buf;
					rsc = l->resource;
					switch (l->m_type) {
					    case cacheLine::TYPE_FILEREF:
						file = rsc->CachedFile();
						if (stat (file, &buf) >= 0 && (buf.st_mode & S_IFREG) == S_IFREG) {
							if (rsc->Size() == 0)
								rsc->SetSize(buf.st_size);
							trace (DEBUG_CACHE) {
								fprintf (stderr, "Cache: Url %s found in file %s\n", urlAbsolute, file);
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
						rsc = NULL;
						l = NULL;
						/* Not ready */
						break;
					}
				} else {
					trace (DEBUG_CACHE)
						fprintf (stderr, "Cache: Url %s has expired\n", urlAbsolute);
				}
			}
		}
		l = FindNextCacheLine(urlAbsolute, l);
	}

	return rsc;
}

Cache::cacheLine *Cache::FindNextCacheLine(const char *url, Cache::cacheLine *current) {
	cacheLine *l = current ? current->next : NULL;
	while (l) {
		if (l->url.Str() && !strcmp(l->url.Str(), url)) {
			trace (DEBUG_CACHE)
				printf("FindNextCacheLine : %s has another entry\n", url);
			break;
		}
		l = l->next;
	}
	return l;
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

bool Cache::AddResource(const char *url, Resource *rsc, cacheLine *l) {
	//Add this resource in the cache

	bool created = false;
	bool empty = false;

	if (!l) {
		// if not cacheLine, find url in cache
		l = FindCacheLine(url);
	}
	if (!l) {
		// If no matching cache line found, create a new one
		nbLine++;
		created = true;
		l = new cacheLine;
		l->int1 = 0;
		l->next = NULL;
		l->url.SetToDup (url);
		l->resource = NULL;
		l->m_type = cacheLine::TYPE_NONE;
		l->m_state = cacheLine::STATE_PENDING;
	}
	if (rsc) {
		if (l->resource) {
			l->Remove();
		}
		l->resource = rsc;
		l->m_type = cacheLine::TYPE_MEMORY;
		l->m_state = cacheLine::STATE_OK;
	} else {
		empty = created ;
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
	time_t ifModifiedSince = 0;
	bool found = false;
	bool needValidate = false;

	StrRef ref;
	url->ToStrRef(&ref);
	const char *urlAbsolute = ref.Str();

	cacheLine *l = FindCacheLine(urlAbsolute);
	while (!found && l) {
		if (!l->resource) {
			trace (DEBUG_CACHE)
				fprintf (stderr, "Cache::Retrieve cacheLine matches but no resource\n");
		} else {
			found = true;
			rsc = l->resource;
			trace (DEBUG_CACHE)
				fprintf (stderr, "Cache::Retrieve FOUND url %s\n", urlAbsolute);
			needValidate = l->NeedValidate() || l->m_state == cacheLine::STATE_VALIDATING;
		}
		if (!found)
			l = FindNextCacheLine(urlAbsolute, l);
	}

	if (l && needValidate) {
		trace (DEBUG_CACHE)
			fprintf (stderr, "Cache::Retrieve url %s has (old) resource\n", urlAbsolute);
		if (async && l->m_state == cacheLine::STATE_VALIDATING) {
			// If this resource is already being re-validated, return.
			return NULL;
		} else {
			l->m_state = cacheLine::STATE_VALIDATING;
		}
		ifModifiedSince = rsc->m_modified;
		rsc = NULL;
	}

	const Protocol *proto = !rsc ? url->Protocol() : NULL;

	if (!rsc && (!proto->hasHost || Pref::Default.Online())) {
		// The resource is not in our cache, we need to call 'getter' method
		// this protocol. This is only possible if we are online OR this
		// is a 'local' protocol such as 'file:'

		if (!async) {
				if (proto->protocolGetter) {
					trace (DEBUG_FILE)
						printf("Cache->%s for url %s ...\n", proto->name, urlAbsolute);
					int status = proto->protocolGetter(url, &rsc, ifModifiedSince);
					if (status) {
						fprintf(stderr, "%s error when downloading : %s\n", proto->name, urlAbsolute);
						if (l) {
							// XXX I guess that in certain circonstancies
							// we may want to revalidate existing resources
							// eg. when offline
							l->m_state = cacheLine::STATE_FAILED;
						}
					}
				}
				if (rsc) {
					//Add this new resource in the cache
					AddResource(urlAbsolute, rsc, l);
					if (!strnull(rsc->m_location)) {
						StrRef r;
						r.SetToDup(rsc->m_location);
						url->SetDisplayName(&r);
					}
				} else {
					if (l && l->resource!=NULL) {
						// The old resource is still ok
						l->m_state = cacheLine::STATE_OK;
						rsc = l->resource;
						rsc->m_date = time(0);
						trace (DEBUG_CACHE)
							fprintf(stderr, "Url %s re-validated\n", urlAbsolute);
					}
				}
		} else {
			if (l != NULL || AddResource(urlAbsolute, NULL)) {
				if (Pref::Default.Online()) {
					trace (DEBUG_CACHE)
						printf("Cache::Retreive queueing url %s\n", urlAbsolute);
					BMessage msg (GET_IMAGE);
					msg.AddString ("url", urlAbsolute);
					msg.AddPointer("urlObject", url);
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
	const struct Protocol *proto;
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
	for (proto = protocol_list; proto->name!=NULL; proto++) {
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
		for (proto = protocol_list; proto->name!=NULL; proto++) {
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

	// As soon as the Url object is created we ask the HTTP getter to
	// download it. I am not sure this is really good.
	m_resource = Cache::cache.Retrieve (this, async, reformat);

	if (!m_resource && !async) {
		fprintf (stderr, "Synchronous url %s not found\n", m_urlStrRef.Str());
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

int httpGetter(Url *url, Resource** rsc, time_t ifModifiedSince) {
	*rsc = NULL;

	ServerConnection *server =
		ConnectionMgr::Default.GetConnection (url);

	if (server) {
		int protocol_result;
		server->OpenConnection (url);
		server->PrepareHeader (url, ifModifiedSince);
		server->SendHeader (url);
		server->ReadHeader (&protocol_result);

		switch (protocol_result)
		{
		case 204:
		case 304:	// not modified
			// XXX Code 204, 304 => no body but why ?
			break;
		default:
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
		if (*rsc || protocol_result==304)
			return 0;
		else 
			return -1;
	} else {
		fprintf (stderr, "Could not get connection to Server\n");
		return -1;
	}
}

int fileGetter(Url *url, Resource **rsc, time_t) {
	* rsc = new FileResource(url->File());
	if (*rsc) return 0;
	else	  return -1;
}
