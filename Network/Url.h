#ifndef URL_H
#define URL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#ifdef __BEOS__
#include <DataIO.h>
#endif
#include <sys/param.h>
#include "platform.h"

#include "UrlQuery.h" // XXX ugly
class Resource;
class ResourceGetter;
struct Protocol;

class Url {
// url fields
	const struct Protocol *m_protocol;
	char m_host[MAXHOSTNAMELEN];
	unsigned int m_port;
	char m_file[FILENAME_MAX];
	char m_target[FILENAME_MAX]; 	// This is the anchor target within the page
					// ie : some_url.html#target
	ProtocolMethod m_method;
	int m_nbParameters;
	char **m_parameters;			// parameters (forms)

	Resource *m_resource;
	bool m_error;
	void _SetTo(const UrlQuery *urlQuery, const Url* base, bool aync=false, bool reformat=false);
	void ToAbsolute(const Url *basUrl);	// Convert a relative url to absolute
	StrRef m_urlStrRef;		// String ref representing absolute url form
	StrRef m_displayName;

public:
	Url (const UrlQuery *urlQuery, const Url* base, bool aync=false, bool reformat=false);
	Url (const char *urlName, const Url* base, bool aync=false, bool reformat=false);
	~Url ();
	void CopyFrom(const Url*);	// This is like a copy-ctor but I don't like copy-ctor
	void SetError();
	Resource *GetDataNow();
	Resource *GetIfAvail();

/*
 * These are getters for different url parts
 * They copy the requested part at the end of the given string
 * and return the number of char written
 */
 	const char *File() const;
	int FilePart(char *str) const;		// prints the file part
	int HostAndPortStr(char *str) const;	// prints "host:port" (if needed) or simply "host"
	const char* Host() const;
	unsigned Port() const;
	const struct Protocol* Protocol() const;
	void ToStrRef(StrRef *) const;
	int ToString(char *str) const;		// XXX OBSOLETE prints the whole string, warning result may
						// vary depending on ToAbsolute has been called or not
	int NbParams() const;
	const char *ParamAt(int i) const;
	ProtocolMethod Method() const;

	const StrRef *DisplayName() const { return m_displayName.IsFree() ? &m_urlStrRef : &m_displayName; }
	void SetDisplayName(const StrRef *r) { m_displayName.SetToRef(r); }
};

class Cache {
        struct cacheLine;
        cacheLine *cacheLines;
        int nbLine; // Current number of ressources
        Cache();
        ~Cache();
        ResourceGetter *m_getter;
	cacheLine *FindCacheLine(const char *url);
	bool AddResource(const char *url, Resource *rsc);
public:
        void Init();
        static Cache cache;
        void SetBase(Url *base);
        void SetResourceGetter(ResourceGetter *getter);
        Resource *Find(Url *url);
        Resource *Retrieve(Url *url, bool async, bool reformat);
        void ClearAll();
};

#endif
