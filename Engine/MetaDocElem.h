#ifndef MetaDocElem_H
#define MetaDocElem_H

/* this file defines TagDocElem that are related to
   the document meta data : <TITLE>, <LINK>, ...
*/

#include "TagDocElem.h"
#include "StrDocElem.h"
#include "DocWalker.h"
#include "Html.h"
#include "StrPlus.h"

class Url;

class TITLE_DocElem : public TagDocElem {
	DocElem* hidden;
	char m_title[500];
public:
	TITLE_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
		m_title[0]='\0';
		hidden = NULL;
	}
	virtual void RelationSet(HTMLFrame *view) {
		hidden = m_included;
		m_included = NULL;

		DocWalker walk(hidden);
		DocElem *elem;
		while ((elem = walk.Next()) != NULL) {
			walk.Feed(elem);
			StrDocElem *titleElem;
			if ((titleElem = dynamic_cast<StrDocElem *>(elem))) {
				strcat(m_title, titleElem->str);
			}
		}
		if (m_title[0]!='\0') {
			view->m_document.SetTitleRef(m_title);
		}
		TagDocElem::RelationSet(view);
	}	
	virtual DocElem *Hides() const { 
		return hidden;
	}
};

class BODY_DocElem : public TagDocElem {
public:
        BODY_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
        }
        virtual void geometry(HTMLFrame *view);
        virtual void draw(HTMLFrame *m_view, const BRect *, bool onlyIfChanged=false);
};

class LINK_DocElem : public TagDocElem {
	typedef TagDocElem super;
	Url *m_iconUrl;
public:
	LINK_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
		m_iconUrl = NULL;
	}
	virtual void RelationSet(HTMLFrame *view);
        virtual void dynamicGeometry(HTMLFrame *view);
};

class MetaDocElem : public TagDocElem {
public:
	MetaDocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
	}
	virtual void geometry(HTMLFrame *view);
	virtual void draw(HTMLFrame *m_view, const BRect *, bool onlyIfChanged=false);
	virtual bool IsActive();
	virtual bool Action(::Action action, UrlQuery *href);
private:
	StrRef m_url;
	StrRef m_text;
};

#endif
