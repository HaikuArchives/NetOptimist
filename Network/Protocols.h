#ifndef PROTOCOLS_H
#define PROTOCOLS_H

class Url;
class MemoryResource;

enum ProtocolId {
	HttpId,
	HttpsId,
	FileId,
	MailToId,
	FtpId,
	BoggusId
};

struct Protocol {
	ProtocolId id;
	const char *name;
	bool hasHost;
	unsigned defaultPort;
	int (*protocolGetter)(Url *, Resource **rsc);
};

extern const Protocol protocol_list[];

class ServerConnection {
public:
	virtual bool Match(Url *) const =0;
	virtual void OpenConnection(Url *url) =0;
	virtual void PrepareHeader(Url *url, bool keepalive=true) =0;
	virtual void SendHeader(Url *url) =0;
	virtual int ReadHeader(int *protocol_specific_result) =0;
	virtual void CloseConnection() =0;
	virtual MemoryResource *GetData() =0;
	virtual bool Status(StrRef *reason) = 0;
};

class FileServerConnection : ServerConnection {
	/* This is for handling file:// urls. */
public:
	virtual bool Match(Url *url) const;
	void OpenConnection(Url *url);
	void PrepareHeader(Url *url, bool keepalive=true);
	void SendHeader(Url *url);
	int ReadHeader(int * /*unused*/);
};

#endif
