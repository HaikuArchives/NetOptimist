#ifndef Cookie_h
#define Cookie_h

#include "platform.h"
#include <time.h>
#include <stdio.h>


class Cookie {
        Cookie *m_next;
	/* This class stores Cookies */
	time_t m_creation_date;
	time_t m_modif_date;
	time_t m_expires;
	char *m_domain;
	char *m_name;
	char *m_value;
	char *m_path;
	bool m_secure;  // use only with secure protocols (https)
	// for user privacy
	bool m_use;
	bool m_readOnly;
	static bool hostMatchDomain(const char *host, const char *domain);
public:
	Cookie(const char *str, Cookie *list);
	virtual ~Cookie();
	void SaveToFile(FILE *fp);
	bool match(const char *hostname, const char *url, char buf[], int len);
	static bool parseCookieFromHttp(const char *headerLine, const char *host);
	Cookie *Next() {
		return m_next;
	}

	const char *Name() { return m_name; }
	const char *Value() { return m_value; }
};

class HostAuth {
	/* This class represents per-host (or domain) cookie authorisation */
	char *m_name;	/* host or domain name */

public:
	bool m_acceptCookie;
	bool m_acceptHttp;

	bool Apply(const char *host) const; // return true if host match m_name;
	void save(FILE *fp);
	static HostAuth *Instanciate(FILE *fp);
};

class CookiesMgr {
	Cookie *clist;
	CookiesMgr();
	~CookiesMgr();
public:
	static CookiesMgr Default;
	Cookie *CookiesList();
};

#endif
