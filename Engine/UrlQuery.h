#if !defined(QUERYURL_H)
#define QUERYURL_H

#include "StrPlus.h"

enum ProtocolMethod {
	METHOD_BUGGY,
	METHOD_NORMAL, // Use this for all links (all protocols) except forms
	METHOD_GET,
	METHOD_POST
};


class UrlQuery {
public:
	UrlQuery() {
		m_name = NULL;
		m_method = METHOD_NORMAL;
		m_nbFields = 0;
		m_maxHop = 10;
	}
	~UrlQuery();
	void SetUrl(const char *);
	void SetUrl(StrRef *);
	const char *Url() const ;

	const char *m_name;
	ProtocolMethod m_method;
	int m_maxHop;		// The max url redirect we may cross
	int m_nbFields;
	char **m_fields;
private:
	StrRef m_url;
};

#endif
