#include "Cookies.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

Cookie::Cookie(const char *str, Cookie *list) {
	char domain[200]; // XXX MAXHOSTNAMELEN ?
	char rootpath[100];
	char cookieText[1024*8];
	long temp_expires;

	sscanf(str, "%s %s %ld %s", domain, rootpath, &temp_expires, cookieText);
	m_expires = temp_expires;
	m_domain = strdup(domain);
	m_path = strdup(rootpath);
	m_name = strdup(cookieText);
	m_value = strchr(m_name, '=');
	if (m_value) {
		*m_value = '\0';
		m_value++;
	}
	//printf("d<%s>p<%s>e<%d><%s><%s>\n", m_domain, m_path, m_expires, m_name, m_value?m_value:"(nil)");
	m_next = list;
}

Cookie::~Cookie() {
	free(m_name);
	// don't free(m_value) because m_value is from the same allocation block as m_name
	free(m_domain);
	free(m_path);
}

void Cookie::SaveToFile(FILE *fp) {
	fprintf(fp, "%s %s %d %s=%s\n", m_domain, m_path, (int)m_expires, m_name, m_value?m_value:"");
}

bool Cookie::hostMatchDomain(const char *hostname, const char *domain) {
	int dom_len = strlen(domain);
	int host_len = strlen(hostname);
	int offset = host_len - dom_len;
	if (offset >= 0) {
#ifdef XXX_strict_cookies_parsing
		/* Domain must either be prefixed by a dot or exactly match the hostname */
		/* XXX We should verify that host prefix doesn't contain `.'
			in regard to domain.
			eg domain .abc.com is ok for host x.abc.com
			but not for host y.z.abc.com (domain should be z.abc.com)
			This is not in the original Netscape specification but
			is in rfc 2965
		*/
		return (offset==0||domain[0]=='.') && strcasecmp(hostname + offset, domain)==0;
#else
		return strcasecmp(hostname + offset, domain)==0;
#endif
	}
	return false;
}

bool Cookie::match(const char *hostname, const char *url_file, char buf[], int len) {
	if (strstr(url_file, m_path)==url_file) {
		// m_path is a prefix for url_file
		if (hostMatchDomain(hostname, m_domain)) {
			if (len>0) {
				snprintf(buf, len, "Cookie: %s=%s", m_name, m_value);
			}
			return true;
		}
	}
	if (hostMatchDomain(hostname, m_domain)) {
		printf("could have been : %s != %s\n", url_file, m_path);
	}
	return false;
}

bool Cookie::parseCookieFromHttp(const char *headerLine, const char *host) {
	const char setCookieStr[] = "Set-Cookie:";
	const char expireStr[] = "expires=";
	const char domainStr[] = "domain=";
	const char pathStr[] = "path=";
	const char secureStr[] = "secure";

	const char *ptr = headerLine;

	int strLen;
	char *name = NULL;
	char *value = NULL;
	char *path = NULL;
	char *domain = NULL;
	bool secure = false;

	// skip spaces
	while (*ptr && *ptr==' ') ptr++;
	// skip http header
	if (!strncasecmp(setCookieStr, ptr, sizeof(setCookieStr)-1)) {
		ptr += sizeof(setCookieStr)-1;
	}

	while (*ptr) {
		while (ptr && (*ptr==' '||*ptr==';')) ptr++;	// skip spaces
		if (!name) {
			strLen = 0;
			while (*(ptr+strLen) && *(ptr+strLen)!='=') strLen++; // seach `=' sign
			name = (char *)malloc(strLen+1);
			strncpy(name, ptr, strLen);
			name[strLen] = '\0';
			ptr+=strLen;

			if (*ptr=='=') ptr++;

			strLen=0;
			while (*(ptr+strLen) && *(ptr+strLen)!=';' && *(ptr+strLen)!=' ')
				strLen++;
			value = (char *)malloc(strLen+1);
			strncpy(value,ptr,strLen);
			value[strLen] = '\0';
			ptr+=strLen;

		} else {
			if (!strncasecmp(secureStr, ptr, sizeof(secureStr)-1)) {
				ptr += sizeof(secureStr)-1;
				secure = true;
				continue;
			} else if (!path && !strncasecmp(pathStr, ptr, sizeof(pathStr)-1)) {
				ptr += sizeof(pathStr)-1;

				strLen=0;
				while (*(ptr+strLen) && *(ptr+strLen)!=';' && *(ptr+strLen)!=' ')
					strLen++;
				path = (char *)malloc(strLen+1);
				strncpy(path,ptr,strLen);
				path[strLen] = '\0';
				ptr+=strLen;

				continue;
			} else if (!domain && !strncasecmp(domainStr, ptr, sizeof(domainStr)-1)) {
				ptr += sizeof(domainStr)-1;

				strLen=0;
				while (*(ptr+strLen) && *(ptr+strLen)!=';' && *(ptr+strLen)!=' ')
					strLen++;
				domain = (char *)malloc(strLen+1);
				strncpy(domain,ptr,strLen);
				domain[strLen] = '\0';
				ptr+=strLen;

				continue;
			}
			ptr++;
		}
	}
	if (!domain) {
		domain=strdup(host);
	}
	if (host) {
		/* Here we verify that the cookie is not violating the user privacy */
		if (!Cookie::hostMatchDomain(host, domain)) {
			fprintf(stderr, "Cookie %s=%s was rejected because domain %s is not valid suffix for host %s\n",
				name, value, domain?domain:"(nil)", host);
			return false;
		}
	}

	printf("COOKIE:: %s=%s ;domain=%s;path=%s;secure=%s\n", name, value, domain?domain:"(nil)", path?path:"(nil)", secure?"true":"false");
	return true;
}


CookiesMgr::CookiesMgr() {
	clist = NULL;

	FILE *cf = fopen("Cookies.data", "r");
	if (cf == NULL) {
		perror("Could not read cookie file");
		return;
	}

	const int maxlen = 512*16;
	char line[maxlen];

	while ( fgets(line, maxlen, cf) != NULL ) {
		if (line[0]=='#') continue;
		clist = new Cookie(line, clist);
	}
	fclose(cf);
}

CookiesMgr::~CookiesMgr() {
	Cookie *next;
	FILE *cf = NULL;
	
	if (clist)
		cf = fopen("Cookies.out", "a");
	while (clist) {
		clist->SaveToFile(cf);
		next = clist->Next();
		delete clist;
		clist = next;
	}
	if (cf) fclose(cf);
}

Cookie * CookiesMgr::CookiesList() {
	return clist;
}

CookiesMgr CookiesMgr::Default;

#if STDA /* This is for test only */
int main() {
	Cookie *clist = CookiesMgr::Default.CookiesList();

	for (Cookie *cur = clist; cur != NULL; cur = cur->Next()) {
		char buf[4096];
		if (cur->match("newsone.net", "/nnr", buf, sizeof(buf)-1))
			printf("1 matches %s\n", buf);
	}

	Cookie::parseCookieFromHttp("Set-cookie: truc=bidule ;;d;domain=.web.org", "www.abc.com");
	Cookie::parseCookieFromHttp("Set-cookie: truc=bidule ; path=/;", "www.abc.com");
	Cookie::parseCookieFromHttp("Set-cookie: securecookie=bidule ;secure;", "www.abc.com");
	Cookie::parseCookieFromHttp("Set-cookie: domcookie=bidule; domain=.abc.com ;secure;", "www.abc.com");
}
#endif
