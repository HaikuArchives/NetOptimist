#ifndef VariousDocElem_H
#define VariousDocElem_H

#include "TagDocElem.h"
#include "StrDocElem.h"
#include "DocWalker.h"
#include "Constraint.h"
#include "Html.h"
#include "Action.h"

class Url;

class SCRIPT_DocElem : public TagDocElem {
	DocElem* hidden;
public:
	SCRIPT_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
		hidden = NULL;
	}
	virtual void RelationSet(HTMLFrame *view) {
		/*
		hidden = m_included;
		m_included = NULL;
		*/
		TagDocElem::RelationSet(view);
	}
	virtual DocElem *Hides() const {
		return hidden;
	}
	virtual void geometry(HTMLFrame *view);
};

class NOSCRIPT_DocElem : public TagDocElem {
	DocElem* hidden;
public:
	NOSCRIPT_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
		hidden = NULL;
	}
	virtual void RelationSet(HTMLFrame *view);
	virtual DocElem *Hides() const {
		return hidden;
	}
};

class STYLE_DocElem : public TagDocElem {
	DocElem* hidden;
public:
	STYLE_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
	}
	virtual void RelationSet(HTMLFrame *view) {
		hidden = m_included;
		m_included = NULL;
		TagDocElem::RelationSet(view);
	}
	virtual DocElem *Hides() const {
		return hidden;
	}
};

class A_DocElem : public TagDocElem {
	// This tag may be a link OR a target !!!!
	StrRef attr_href;
	StrRef m_attr_name;
	StrRef m_attr_alt;
	StrRef m_attr_onclick;
public:
	A_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
	}
	virtual void geometry(HTMLFrame *view);
	virtual bool IsActive();
	virtual bool Action(::Action action, UrlQuery *href);
};

class HR_DocElem : public TagDocElem {
	typedef TagDocElem super;
public:
	HR_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
	}
	virtual void draw(HTMLFrame *m_view, bool onlyIfChanged=false);
	virtual void dynamicGeometry(HTMLFrame *view);
	virtual void geometry(HTMLFrame *view);
};


class LIST_DocElem : public TagDocElem {
public:
	LIST_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
	}
	virtual void geometry(HTMLFrame *view);
};

class LI_DocElem : public TagDocElem {
public:
	LI_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
	}
	virtual void geometry(HTMLFrame */*view*/) {
		w=h=10;
	}
	virtual void draw(HTMLFrame *m_view, bool onlyIfChanged=false);
};


class FrameDocElem : public TagDocElem {
	StrRef m_text;
	StrRef m_href_attr;
	StrRef m_alt_attr;
	char m_name_attr[50];
	const char *UserName() const;
public:
	FrameDocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
		m_name_attr[0] = '\0';
	}
	virtual void geometry(HTMLFrame *view);
	virtual void draw(HTMLFrame *m_view, bool onlyIfChanged=false);
	virtual bool IsActive();
	virtual bool Action(::Action action, UrlQuery *href);
};


class IMG_DocElem : public TagDocElem {
	StrRef m_attr_alt;
	StrRef m_attr_src;
	int	attr_hspace;
	int	attr_vspace;
	int	attr_border;
	Alignment attr_align;
	BBitmap *bmp;
	bool badImage;
	int imgW, imgH;
	int reqW, reqH;
	Url *url;
	void LoadImage();
	void GetSize();
	typedef TagDocElem super;
public:
	IMG_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
		bmp = NULL;
		url = NULL;
		imgH = imgW = -1;
		reqH = reqW = -1;
		badImage = false;
		attr_hspace = 0;
		attr_vspace = 0;
		attr_border = 1;
	}
	~IMG_DocElem();
	virtual void geometry(HTMLFrame *view);
	virtual void dynamicGeometry(HTMLFrame *view);
	virtual void draw(HTMLFrame *m_view, bool onlyIfChanged=false);
};

#endif
