#ifndef Table_H
#define Table_H

#include <assert.h>

#include "TagDocElem.h"
#include "Constraint.h"
#include "Event.h"

class TableDocElem;
class TR_DocElem;
class TD_DocElem;
class TableDim;

class TableDocElem : public TagDocElem, public EventProcessor {
	TableDim *dims;
	void PlaceChildren();
	struct TR_List {
		struct TD_List {
			TD_DocElem* elem;
			TD_List* next;
		};
		TR_DocElem* elem;
		TR_List* next;
		TD_List* cells;
	};
	TR_List *contentList;
	void computeSize();
	int m_attr_height;
public:
	int borderWidth;		// HTML : "border"
	int cellPaddingWidth;	// HTML : "cellpadding"
	int cellSpacingWidth;	// HTML : "cellspacing"
	Alignment attr_align;
	Events events;
	TableDocElem(Tag *tag, TagAttr* attrList = NULL);
	~TableDocElem();
	virtual bool update(int , int , int bottom);
	virtual void geometry(HTMLFrame *view);
	virtual void dynamicGeometry(HTMLFrame *view);
	virtual void place();
	virtual void draw(HTMLFrame *m_view, bool onlyIfChanged=false);
	virtual void initPlacement();
	void RegisterLine(TR_DocElem*);
	void RegisterCell(TD_DocElem*);
	virtual void updateCol(int line, int col, int h);
	virtual void updateLine(int line, int col, int h);

	void Process(int evt);
};

class TR_DocElem : public TagDocElem {
public:
	TableDocElem *container;
		/* This field points to the Table container of this TR */
	TR_DocElem(Tag *tag, TagAttr* attrList = NULL) : TagDocElem(tag, attrList) {
		container = NULL;
	}
	virtual void geometry(HTMLFrame *view);
	virtual void place() {
		printPosition("TR_place");
	}
	virtual void RelationSet(HTMLFrame *view);
};

class TD_DocElem : public TagDocElem /* WARNING : super used */ {
	int m_attr_height;
	Alignment attr_align;
public:
	int colspan;
	int rowspan;
	int line,col;
		/* This field points to the TABLE containing this TD */
	TableDocElem *container;

	TD_DocElem(Tag *tag, TagAttr* attrList = NULL) : TagDocElem(tag, attrList) {
		line = col = -1;
		colspan = 1;
		rowspan = 1;
		x = y = -1;
		includedConstraint = new Constraint(this);
		container = NULL;
		m_attr_height = -1;
	}
	virtual ~TD_DocElem() { }
	virtual void draw(HTMLFrame *m_view, bool onlyIfChanged=false);
	virtual bool update(int right, int top, int bottom);
	virtual void geometry(HTMLFrame *view);
	virtual void place();
	virtual void RelationSet(HTMLFrame *view);
	virtual const char * printSpecific() {
		// XXX to be removed
		static char tmp[100]; // XXX must be removed
		sprintf(tmp, "col %d line %d colspan %d", col, line, colspan);
		return tmp;
	}
};
#endif
