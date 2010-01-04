#ifndef TagDocElem_H
#define TagDocElem_H

#include "TagAttr.h"
#include "Tag.h"
#include "DocElem.h"
#ifdef __BEOS__
#include <be/support/Debug.h>
#include <assert.h>
#endif


class TagDocElem : public DocElem {
	friend class DocFormater;
	Tag *t;
protected:
	Style *m_includedStyle;
	TagAttr *list;
public:
	const char *toString() const { return (t->toString()); }
	TagDocElem(Tag *symbol, TagAttr* attrList = NULL) : DocElem() {
		ASSERT(h<5000);
		t=symbol;
		m_included=NULL;
		list = attrList;
		m_includedStyle = NULL;
	}
	~TagDocElem();
	bool close(const Tag *symbol, DocElem *last);
	bool isClosing() { return t->closing; }
	/*const ?*/ TagAttr *AttributeList() { return list; }
	const Tag *tag() { return t; }
	virtual void place();
	virtual void draw(HTMLFrame *m_view, const BRect*, bool onlyIfChanged=false);
	virtual void ClosingParent(DocElem *openningTag) {
		if (!isClosing())
			fprintf(stderr, "ERROR : try to set matching openning tag for a non-closing tag : id=%d\n", id);
		ASSERT(isClosing());
		m_up = openningTag;
	}
	virtual const Style *ChildrenStyle() const {
		if (m_includedStyle)
			return m_includedStyle;
		else
			return m_style;
	}
};

#endif
