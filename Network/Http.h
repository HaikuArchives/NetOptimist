#ifndef HTTP_H
#define HTTP_H

#ifdef __BEOS__
#include <sys/socket.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "Protocols.h"

class Resource;
class MemoryResource;
class BMallocIO;

#define HTTP_EOL	"\r\n"
#define HTTP_VERSION	"HTTP/1.1"
#define HTTP_USER_AGENT	"NetOptimist"
#define DEFAULT_HTTP_PORT 80

extern const char *httpKeywordList[];

class HttpServerConnection : public ServerConnection {
	char *m_host;
	unsigned int m_port;
	int protocol; // http/https
	int version;
	int state;
	char header[4096];
// Connection : 
	bool m_error;	// something is wrong
	StrRef m_reason;
	int m_socket;
	struct sockaddr_in m_serverAddr;
	bool m_useProxy;
// Protocol
	bool m_isChunked;
	bool m_isClosed;   // The connection is closed or will be closed after the current transfert
	void ParseHeader(const char *keywork, char *value);
// Data
	size_t m_dataSize;	// Data size value from http header
	MemoryResource *m_memoryResource;
	BMallocIO *m_stream;
	const char *Host();
	unsigned int Port();
	int ReadHttpStatus(int * http_result);
	int EncodeUrl(char *dest, const char *source);
public:
	bool Match(Url *) const;
	HttpServerConnection(struct sockaddr_in connectTo, bool useProxy);
	virtual ~HttpServerConnection() {}
	const char *MethodStr(ProtocolMethod);
	void OpenConnection(Url *url);
	void PrepareHeader(Url *url, time_t ifModifiedSince, bool keepalive=true);
	void SendHeader(Url *url);
	int ReadHeader(int *http_code);
	MemoryResource *GetData();
	bool IsClosed() const;
	bool Status(StrRef *reason);
	void CloseConnection();
};

class ConnectionMgr {
	int m_nbConnectionPending;
	int m_maxConnections;
	ServerConnection **m_conns;

	ConnectionMgr(int maxConnections = 10);
	~ConnectionMgr();
public:
	static ConnectionMgr Default;
	ServerConnection *GetConnection(Url *url);
	void ReleaseConnection(ServerConnection *connection);
};

#endif
