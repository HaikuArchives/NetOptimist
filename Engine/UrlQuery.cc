#include <stdio.h>
#include "UrlQuery.h"
#include "StrPlus.h"

void UrlQuery::SetUrl(StrRef *url) {
	m_url.SetToRef(url);
}

void UrlQuery::SetUrl(const char *url) {
	m_url.SetToDup(url);
}

const char *UrlQuery::Url() const {
	return m_url.Str();
}

UrlQuery::~UrlQuery() {
}
