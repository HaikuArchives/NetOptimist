#ifndef TextDocElem_H
#define TextDocElem_H

#include "TagDocElem.h"
#include "Style.h"

class FONT_DocElem : public TagDocElem {
	int attr_size;
	char attr_font[30];
public:
	FONT_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
		attr_font[0]='\10';
		attr_size=11;
	}
	virtual void geometry(HTMLFrame *view);
};

class I_DocElem : public TagDocElem {
public:
	I_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
	}
	virtual void geometry(HTMLFrame *view);
};

class U_DocElem : public TagDocElem {
public:
	U_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
	}
	virtual void geometry(HTMLFrame *view);
};

class B_DocElem : public TagDocElem {
public:
        B_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
        }
        virtual void geometry(HTMLFrame *view);
};

class BLOCKQUOTE_DocElem : public TagDocElem {
public:
        BLOCKQUOTE_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
        }
        virtual void geometry(HTMLFrame *view);
};

class H1_DocElem : public TagDocElem {
public:
        H1_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
        }
        virtual void geometry(HTMLFrame *view);
};

class H2_DocElem : public TagDocElem {
public:
        H2_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
        }
        virtual void geometry(HTMLFrame *view);
};

class H3_DocElem : public TagDocElem {
public:
        H3_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
        }
        virtual void geometry(HTMLFrame *view);
};

class BIG_DocElem : public TagDocElem {
public:
        BIG_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
        }
        virtual void geometry(HTMLFrame *view);
};

class SMALL_DocElem : public TagDocElem {
public:
        SMALL_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
        }
        virtual void geometry(HTMLFrame *view);
};

class FIXED_DocElem : public TagDocElem {
public:
        FIXED_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
        }
        virtual void geometry(HTMLFrame *view);
};

#endif
