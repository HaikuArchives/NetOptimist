#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "StrPlus.h"

class Url;
class BBitmap;

class Document {
	const char *title;
	int	m_nbLinks;
	StrRef	m_linkNameTab[10];
	StrRef	m_linkUrlTab[10];
	Url *m_url;
	BBitmap *m_icon;
public:
	void SetUrl(Url *url) {
		m_url = url;
		title = NULL;
		m_nbLinks = 0;
		m_icon = NULL;
	}
	class Url *CurrentUrl() { return m_url; }

	void SetIcon(BBitmap *bmp) { m_icon = bmp; } // XXX should detroy m_icon, dtor too !
	const BBitmap *Icon() const { return m_icon; }

	Document() {
		SetUrl(NULL);
	}
	const char* TitleRef() {
		return title;
	}
	void SetTitleRef(const char *ref) {
		title = ref;
	}
	void ClearLinks() {
		for (int i = 0; i<m_nbLinks; i++) {
			m_linkNameTab[i].Free();
			m_linkUrlTab[i].Free();
		}
		m_nbLinks = 0;
	}
	void AddLink(StrRef *name, StrRef *url) {
		if (m_nbLinks<10) {
			m_linkNameTab[m_nbLinks].SetToRef(name);
			m_linkUrlTab[m_nbLinks].SetToRef(url);
			m_nbLinks++;
		}
	}
	bool GetLinkAt(int position, StrRef *name, StrRef *url) const {
		if (position < m_nbLinks && !m_linkNameTab[position].IsFree()) {
			name->SetToRef(&m_linkNameTab[position]);
			url->SetToRef(&m_linkUrlTab[position]);
			return true;
		} else {
			return false;
		}
	}
};

#endif
