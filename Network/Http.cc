
#include "Url.h"
#include "UrlQuery.h"
#include "Protocols.h"
#include "Http.h"
#include "Resource.h"
#include "traces.h"
#include "Cookies.h"
#include "StrPlus.h"
#include <errno.h>

#if !defined(__BEOS__) || defined(BONE_VERSION) 
# include <arpa/inet.h>
#endif
#include <unistd.h>
#if defined(__BEOS__)
extern "C" const char	*strptime(const char *buf, char *fmt, struct tm *tm);
#endif

const char *httpKeywordList[] = {
	/* This list must be kept in sync with the HttpKeywordIds enum */
	"Cache-Control",
	"Status",
	"Set-Cookie",
	"Location",
	"Connection",
	"Keep-Alive",
	"Content-Length",
	"Content-Location",
	"Content-Type",
	"Date",
	"Expires",
	"Last-Modified",
	"WWW-Authenticate",
	"Via",
	"Server",
	"Transfer-Encoding",
	"Age",
	"Accept-Ranges",
	"Etag",
	"X-Powered-By",
	"Warning",
	"P3P",
	NULL
};

enum HttpKeywordIds {
	HttpHdrCacheControl,
	HttpHdrStatus,
	HttpHdrSetCookie,
	HttpHdrLocation,
	HttpHdrConnection,
	HttpHdrKeepAlive,
	HttpHdrContentLength,
	HttpHdrContentLocation,
	HttpHdrContentType,
	HttpHdrDate,
	HttpHdrExpires,
	HttpHdrLastModified,
	HttpHdrWWWAuthenticate,
	HttpHdrVia,
	HttpHdrServer,
	HttpHdrTransferEncoding,
	HttpHdrAge,
	HttpHdrAcceptRanges,
	HttpHdrEtag,
	HttpHdrXPoweredBy,
	HttpHdrWarning,
	HttpHdrP3P
};

static time_t dateStrToNumber(const char *datestr, const char *dateNameForDebug=NULL) {
	struct tm tm;
	memset (&tm, 0, sizeof(tm));
	// XXX not enough : see http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.1
	if (strptime(datestr, "%a, %d %b %Y %H:%M:%S", &tm)==NULL) {
		fprintf(stderr, "ERROR: unknown http date format: %s\n", datestr);
		return 0;
	} else {
		time_t age = mktime(&tm);
		trace(DEBUG_HTTP_DATE) {
			if (dateNameForDebug) {
				char redate[100];
				asctime_r(&tm, redate);
				fprintf(stderr, "COMPUTED DATE %s IS %ld, %s", dateNameForDebug, age, redate);
			}
		}
		return age;
	}
}

const char *HttpServerConnection::MethodStr(ProtocolMethod method) {
	switch(method) {
		case METHOD_GET:
		case METHOD_NORMAL:
			return "GET";
		case METHOD_POST:
			return "POST";
		default:
			fprintf(stderr, "ERROR : HttpServerConnection unknown method : %d\n", method);
			return "GET"; // be nice with the server ;-)
	}
}

HttpServerConnection::HttpServerConnection(struct sockaddr_in connectTo, bool useProxy) {
	m_socket = -1;
	m_error = false;
	m_isChunked = false;
	m_isClosed = true;
	m_serverAddr = connectTo;
	m_useProxy = useProxy;
	m_memoryResource = NULL;
}

bool HttpServerConnection::Match(Url *url) const {
	if (m_useProxy) return true;		// XXX Not so simple I guess
	return (
		url->Protocol()->id == HttpId
	&&	url->Port() == m_port
	&&	!strcmp(url->Host(), m_host));
}

void HttpServerConnection::OpenConnection(Url *url) {
	m_stream = NULL;
	m_dataSize = 0;
	
	if (!m_isClosed) {
		// if the connection is still alive, we may reuse it.
		if (m_useProxy) {
			// We assume that the proxy can handle all urls
			return;
		} else {
			// No proxy is use. So this is a direct connection.
			// Verfify that host and port match
			if (m_port ==  url->Port())
				return;
			// XXX How can we verify Host ?
		}
	}
	CloseConnection();
		
	m_port = url->Port();
	m_host = url->Host();
	
	m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket<0) {
		perror( "socket" );
		m_socket = -1;
		m_reason.SetToConst("Network error : could not create socket");
		m_error = true;
		return;
	}
	if(::connect(m_socket, (struct sockaddr*)(&m_serverAddr), sizeof(m_serverAddr))) {
		perror( "connect" );
#if defined(__BEOS__) && !defined(BONE_VERSION)
		closesocket(m_socket);
#else
		close(m_socket);
#endif
		m_socket = -1;
		if (m_useProxy) {
			m_reason.SetToConst("Could not connect to proxy");
		} else {
			m_reason.SetToConst("Could not connect to server");
		}
		m_error = true;
		return;
	}
	m_isClosed = false;
	trace(DEBUG_HTTP) printf("-- OpenConnection error = %d\n", m_error);
}

unsigned int HttpServerConnection::Port() {
	return m_port;
}

const char* HttpServerConnection::Host() {
	return m_host;
}

void HttpServerConnection::CloseConnection() {
	if (m_socket>=0) {
#if defined(__BEOS__) && !defined(BONE_VERSION)
		closesocket(m_socket);
#else
		close(m_socket);
#endif
		m_isClosed = true;
		m_socket = -1;
	}
	m_error = false;
	m_reason.Free();
}

int HttpServerConnection::EncodeUrl(char *dest, const char *input) {
	trace(DEBUG_HTTP) {
		printf("Encoding parameter '%s'\n", input);
	}
	int count = 0;
	while(*input) {
		if (*input == ' ') {
			strcpy(dest+count, "%20");
			count += 3;
		} else {
			dest[count] = *input;
			count++;
		}
		input++;
	}
	dest[count] = '\0';
	trace(DEBUG_HTTP) {
		printf("Encoding result '%s'\n", dest);
	}
	return count;
}

void HttpServerConnection::PrepareHeader(Url *url, bool keepalive) {
	// XXX We should be able to issue 1.0 queries here.
	if (m_error) return;
	ProtocolMethod method = url->Method();
	header[0] = '\0';
	char *ptr = header;

	m_memoryResource = NULL; // Do not reuse previous data

	// The main query "GET /thefile HTTP/1.1"
	ptr += sprintf(ptr, "%s ", MethodStr(method));
	if (!m_useProxy) {
		ptr += url->FilePart(ptr);
	} else {
		// The URL format is different when using proxy or not
		ptr += url->ToString(ptr);
	}
	if (method == METHOD_GET) {
		int nbParams = url->NbParams();
		for (int i = 0; i<nbParams; i++) {
			*ptr = i==0 ? '?' : '&';
			ptr++;
			ptr += EncodeUrl(ptr, url->ParamAt(i));
		}
	}
	ptr += sprintf(ptr, " %s" HTTP_EOL, HTTP_VERSION);

	// Host (for virtual hosts)
	strcat(ptr, "Host: "); ptr += 6;
	ptr += url->HostAndPortStr(ptr);
	ptr = strcat(ptr, HTTP_EOL); ptr += 2;
	
	// Optional header fields

	// Keepalive
	const char *keepaliveStr;
	keepaliveStr = keepalive ? "Keep-Alive" : "Close";
	ptr += sprintf(ptr, "Connection: %s" HTTP_EOL, keepaliveStr);

	// User agent
#ifdef __BEOS__
	ptr += sprintf(ptr, "User-Agent: %s" HTTP_EOL, HTTP_USER_AGENT);
#endif

	// XXX Accept-___ http header fields are not supported
	//ptr += sprintf(ptr, "Accept-Charset: %s" HTTP_EOL, );
	//ptr += sprintf(ptr, "Accept-Language: %s" HTTP_EOL, );
	//ptr += sprintf(ptr, "Accept-Encoding: x-gzip; q=1.0, x-deflate, gzip; q=1.0, deflate, identity\r\n"

	strcat(ptr, HTTP_EOL); // end of header
	trace(DEBUG_HTTP) {
		fprintf(stderr, "----sending-----\n%s\n", header);
	}
}

void HttpServerConnection::SendHeader(Url *url) {
	int len, nbsent;

	if (m_error) return;

	len = strlen(header);
	nbsent = send(m_socket, header, len, 0);
	if (len != nbsent) {
		perror("could not send header");
	}
	// send cookies

	Cookie *clist = CookiesMgr::Default.CookiesList();

	for (Cookie *cur = clist; cur != NULL; cur = cur->Next()) {
		char buf[4096];
		if (cur->match(url->Host(), url->File(), buf, sizeof(buf)-10)) {
			fprintf(stderr, "1 matches %s\n", buf);
			strcat(buf, HTTP_EOL);
			send(m_socket, buf, strlen(buf),0);
		}
	}


	// End of header = empty line
	int nbSent = send(m_socket, HTTP_EOL, strlen(HTTP_EOL),0);
	if (nbSent != (int)strlen(HTTP_EOL)) {
		m_reason.SetToConst("Could not send request to server");
		m_error = true;
	}

	trace(DEBUG_HTTP) if (m_error) printf("-- SendHeader error = %d\n", m_error);
}

int HttpServerConnection::ReadHttpStatus(int * http_result) {
	/* This function must parse the first line of the http response
	 * Format :
	 *	HTTP/1.1 200 OK
	 *	HTTP/1.1 302 Found
	 *	...
	 */
	char headerLine[512];		// Does the rfc recommend values ?
	char *ptr;
	int nbrec;

#ifdef __sun
	memset(headerLine, 0, sizeof headerLine);
#endif
	ptr = headerLine;
	do {
		nbrec = recv(m_socket, ptr, 1, 0);
		ptr++;
	} while (nbrec==1 && *(ptr-1)!='\n');
	*ptr = '\0';
	if (nbrec!=1) {
		trace(DEBUG_HTTP)
			fprintf(stderr, "Http Error :could not read status of http response %s\n", headerLine);
		goto error;
	}
	int skip;
	skip = strprefix(headerLine, "HTTP/1.1");
	if (!skip)
		skip = strprefix(headerLine, "HTTP/1.0");
	if (!skip) {
		trace(DEBUG_HTTP)
			fprintf(stderr, "Http Error :incorrect http response : <%s>\n", headerLine);
		goto error;
	}
	ptr = headerLine + skip;
	while (ptr[0]==' ' || ptr[0]=='\t') ptr++;
	if (sscanf(ptr,"%d", http_result)!=1) {
		trace(DEBUG_HTTP)
			fprintf(stderr, "Http Error :incorrect http response : <%s>\n", headerLine);
		goto error;
	}
	while (ptr[0]!='\0' && ptr[0]!=' ' && ptr[0]!='\t') ptr++;
	// ptr now points to an http string representing status
	trace(DEBUG_HTTP) printf("-- ReadHttpStatus error = %d - %s\n", *http_result, ptr);
	return 0;
error:
	m_reason.SetToConst("Invalid HTTP header from server");
	m_error = true;
	return -1;
}



int HttpServerConnection::ReadHeader(int * http_result) {
	/* This function reads from socket the http header
	   and passes to ParseHeader the header keyword and its value. */

	char headerLine[1024];		// Does the rfc recommend values ?
	char *value;
	char *ptr;
	int nbrec = 0;

	if (m_error) return -1;

#ifdef __sun
	memset(headerLine, 0, sizeof(headerLine));	/* for sun dbx memory checker */
#endif
	if (!m_memoryResource)
		m_memoryResource = new MemoryResource;

	int ret = ReadHttpStatus(http_result);

	if (ret==0 && !m_error) {
		m_memoryResource->m_cacheOnDisk = (*http_result==200);
		if (*http_result!=200)
			trace(DEBUG_HTTP) fprintf(stdout, "HTTP status code is %d\n", *http_result);

		do {
			ptr = headerLine;
			value = NULL;
			do {
				nbrec = recv(m_socket, ptr, 1, 0);
				if (value==NULL && *ptr == ':') {
					// keyword is read, value starting
					*ptr = '\0';
					value = ptr + 1;
				}
				ptr++;
			} while (nbrec==1 && *(ptr-1)!='\n');

			if (value) while (value[0]==' ' || value[0]=='\t') value++;
							// Ignore leading spaces

			ptr--;				// Ignore trailing \n
			if (*(ptr-1)=='\r') ptr--;	// Ignore trailing \r
			*ptr = '\0';

			if (!strnull(headerLine))
				ParseHeader(headerLine, value);
		} while (nbrec >0 && !strnull(headerLine));
	}
	trace(DEBUG_HTTP) if (m_error) printf("-- ReadHeader error = %d\n", m_error);
	return (nbrec>0) ? 0 : -1;
}


////////////////////////////////////////////////////
//	ParseHeader
// 
void HttpServerConnection::ParseHeader(const char *keyword, char *value) {
	const char *ptr;
	int keywordId = 0;
	while ((ptr=httpKeywordList[keywordId]) != NULL) {
		if (!strcasecmp(ptr, keyword))
			break;
		keywordId++;
	}
	switch(keywordId) {
		case HttpHdrTransferEncoding:
			if (strcasecmp(value, "chunked")==0) {
				m_isChunked = true;
			}
			break;
		case HttpHdrLocation:
		case HttpHdrContentLocation:
			if (m_memoryResource && !strnull(value) && strnull(m_memoryResource->m_location))
				m_memoryResource->m_location = strdup(value);
			break;
		case HttpHdrConnection:
			if (strcasecmp(value, "closed")==0) {
				m_isClosed = true;
			}
			break;
		case HttpHdrContentLength:
			m_dataSize = atoi(value);
			trace(DEBUG_HTTP) fprintf(stdout, "Content-Length=%ld\n", m_dataSize);
			break;
		case HttpHdrContentType:
			if (m_memoryResource) {
				// Remove extra information
					// XXX This field also contain information
					// XXX about charset for this page, image quality, ...
					// XXX we should handle that too.
				char *p = value;
				while (*p && *p!=';')
					p++;
				*p = '\0';
				// Store mime type
				m_memoryResource->SetMimeType(value);
			}
			break;
		case HttpHdrDate:
			if (m_memoryResource)
				m_memoryResource->m_date = dateStrToNumber(value, "Date");
			break;
		case HttpHdrExpires:
			if (m_memoryResource)
				m_memoryResource->m_expires = dateStrToNumber(value, "Expires");
			break;
		case HttpHdrLastModified:
			if (m_memoryResource)
				m_memoryResource->m_modified = dateStrToNumber(value, "Last-Modified");
			break;
		case HttpHdrSetCookie:
			//Cookie::parseCookieFromHttp( value, this->Host());
			break;
		case HttpHdrStatus:
		case HttpHdrCacheControl:
		case HttpHdrAge:
		case HttpHdrAcceptRanges:
		case HttpHdrP3P:
		case HttpHdrKeepAlive:
			// we don't handle these because we don't care at the moment !!!
			break;
		case HttpHdrVia:
		case HttpHdrEtag:
		case HttpHdrWarning:
			// ignore
			break;
		case HttpHdrXPoweredBy:
		case HttpHdrServer:
			// Spam !
			break;
		default:
			trace(DEBUG_HTTP) 
				fprintf(stderr, "HTTP WARNING : unknown header: %s VALUE %s\n", keyword, value?value:"(null)");
	}
	trace(DEBUG_HTTP_DUMP_HEADER) 
		fprintf(stdout, "Info http header %s='%s'\n", keyword, value?value:"(null)");
}


////////////////////////////////////////////////////
//	GetData
//
MemoryResource *HttpServerConnection::GetData() {
	const int dataUnitSize = 512 * 3; 
	char dataUnit[dataUnitSize+1];
	long chunkSize, nbrec;
	bool error = false;
	bool end = false;

	if (m_error) return NULL;

	size_t totalReceived=0;
	trace(DEBUG_HTTP) {
		if (m_dataSize==0 && !m_isChunked) 
			fprintf(stderr, "no data size : reading until connection is closed\n");
	}

#ifdef __sun
	memset(dataUnit, 0, sizeof(dataUnit));	/* for dbx memory checker */
#endif
	m_stream = new BMallocIO;
	do {
		if (m_isChunked) {
			// Read 1 line containing chunksize
			char *ptr =	dataUnit;
readoneline:
			do {
				// There is one line containing the chunk size
				// This line is read byte per byte
				// XXX reading 1 char is bad but I don't think it has any
				// XXX performance hit since the line is typically 4 or 5 bytes long
				nbrec = recv(m_socket, ptr, 1, 0);
				end += nbrec==0;
				error += nbrec<0;
				ptr++;
			} while (!error && !end && (ptr==dataUnit ||*(ptr-1)!='\n'));

			if (ptr>dataUnit)
				ptr--;		// Ignore trailing \n
			if (ptr>dataUnit && *(ptr-1)=='\r')
				ptr--;		// Ignore trailing \r
			*ptr = '\0';

			if (!error && !end && dataUnit[0]=='\0') goto readoneline;
			chunkSize = strtol(dataUnit, NULL, 16);
			trace(DEBUG_HTTP_DATA) 
				fprintf(stderr, "ChunkSize = %ld\n", chunkSize);
			end += chunkSize == 0;
		} else {
			if (m_dataSize>0)
				chunkSize = m_dataSize;
			else
				chunkSize = dataUnitSize;
		}
		trace(DEBUG_HTTP_DATA) 
			fprintf(stderr, "HttpServerConnection::GetData we should recv %ld bytes\n", chunkSize);
		while (!error && !end && chunkSize>0) {
			nbrec = recv(m_socket, dataUnit, min(chunkSize,dataUnitSize), 0);
			if (nbrec > 0) {
				trace(DEBUG_HTTP_DATA) 
					fprintf(stderr, "HttpServerConnection::GetData recv %ld bytes\n", nbrec);
				chunkSize -= nbrec;
				totalReceived += nbrec;
				m_stream->Write(dataUnit, nbrec);
			}
			if (nbrec < 0)
				fprintf(stderr, "HttpServerConnection::GetData recv %ld bytes errno 0x%x %s\n", nbrec, errno, strerror(errno));
			end += nbrec==0;
			error = nbrec<0;
		}
		end += !m_isChunked && totalReceived==m_dataSize;
	} while (!error && !end);
	if (error) {
		trace(DEBUG_HTTP)
			printf("-- Warning : an error occured while http'ing data\n");
	}

	m_memoryResource->SetSize(totalReceived);

	if (totalReceived>0) {
		if (!m_memoryResource)
			m_memoryResource = new MemoryResource;
		m_stream->Seek(0, SEEK_SET);
		m_memoryResource->SetData(m_stream);
		trace(DEBUG_HTTP) printf("-- GetData : got %ld bytes total\n", totalReceived);
	} else {
		if (error) {
			m_error = true;
			if (m_reason.IsFree()) {
				m_reason.SetToConst("Error while reading data from server");
			}
		}
		delete m_stream;
		m_stream = NULL;
	}

	trace(DEBUG_HTTP) printf("-- GetData error = %d / size %ld- isCachedFile() %p, isStream %p\n", m_error, totalReceived, m_memoryResource->CachedFile(), m_memoryResource->Data());

	return m_memoryResource;
}


bool HttpServerConnection::Status(StrRef *reason) {
	if (m_error) {
		reason->SetToRef(&m_reason);
		return false;
	}
	return true;
}


#ifdef __BEOS__
ConnectionMgr ConnectionMgr::Default;
#else
ConnectionMgr ConnectionMgr::Default(2);
#endif

ConnectionMgr::ConnectionMgr(int maxConnections) :
	m_maxConnections(maxConnections)
{
	m_conns = new ServerConnection *[m_maxConnections];
	for (int i=0; i<m_maxConnections; i++) {
		m_conns[i]=NULL;
	}
}


ServerConnection* ConnectionMgr::GetConnection(Url *url) {
	for (int i=0; i<m_maxConnections; i++) {
		if (m_conns[i] && m_conns[i]->Match(url)) {
			ServerConnection* conn = m_conns[i];
			m_conns[i]=NULL;
			trace(DEBUG_HTTP) 
				fprintf(stderr, "Reuse connection to : %s:%d\n", url->Host() ,url->Port());
			m_nbConnectionPending++;
			return conn;
		}
	}

	int proxyPort;
	const char *proxy;
	proxy = Pref::Default.HttpProxyName();
	proxyPort = Pref::Default.HttpProxyPort();

	struct hostent *hostinfo;
	struct sockaddr_in serverAddr;
	if (proxy)
		hostinfo = gethostbyname(proxy);
	else
		hostinfo = gethostbyname(url->Host());

	if (hostinfo==NULL) {
		fprintf(stderr, "host %s : ", url->Host());
		perror("unknown host");
		return NULL;
	}
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	if (proxy)
		serverAddr.sin_port = htons(proxyPort);
	else
		serverAddr.sin_port = htons(url->Port());
	serverAddr.sin_addr.s_addr = *(unsigned long*)(hostinfo->h_addr);
	trace(DEBUG_HTTP) 
		fprintf(stderr, "Connecting to : %s:%d\n", inet_ntoa(*(struct in_addr*)(hostinfo->h_addr)),ntohs(serverAddr.sin_port));
	m_nbConnectionPending++;
	return new HttpServerConnection(serverAddr, proxy!=NULL);
}


void ConnectionMgr::ReleaseConnection(ServerConnection *connection) {
	m_nbConnectionPending--;
	for (int i=0; i<m_maxConnections; i++) {
		if (m_conns[i]) {
			m_conns[i]=connection;
			return;
		}
	}
	connection->CloseConnection();
	delete connection;
	trace(DEBUG_HTTP) 
		printf("ConnectionMgr: still %d connections pending\n", m_nbConnectionPending);
}


ConnectionMgr::~ConnectionMgr() {
	for (int i=0; i<m_maxConnections; i++) {
		if (m_conns[i]) {
			m_conns[i]->CloseConnection();
			delete m_conns[i];
		}
	}
}
