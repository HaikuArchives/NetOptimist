#ifndef DocElem_H
#define DocElem_H

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"

#include "DocElemStack.h"
#include "Action.h"

class Tag;
class HTMLFrame;
class Style;
class Constraint;
class UrlQuery;

class DocElem {
	static int idGen;
	friend class DocFormater;
	friend void html_parse(FILE *);
protected: /* XXX BAD */
	DocElem *m_next;
private:
	DocElem *m_prev;
	DocElem *m_last;
public:
	// These are public to avoid writing getters/setters
	int x,y,w,h;
	int fixedW;		// this is the width for static sized elements

	int id;
public:	// not protected because of lack of methods
	Constraint *constraint;
	Constraint *includedConstraint;
protected:
	DocElem *m_up;
	DocElem *m_included;
	const Style *m_style;
	DocElem() {
		id=idGen++;
		m_last = m_included = m_prev = m_next = NULL;
		m_up = NULL;
		fixedW=x=y=w=h=0;
		constraint = NULL;
		includedConstraint = NULL;
		m_style = NULL;
	}
public:
	virtual ~DocElem();
	void add(DocElem *elem, DocElem *last) {
		m_next=elem;
		if (elem) {
			elem->Attached(this, last);
		}
	}
	virtual void Attached(DocElem *previous, DocElem *last) { 
		m_prev = previous;
		m_last = last;
	}
	virtual void Inclosed(DocElem *parent) { 
		m_up = parent;
	}
	virtual const Style *ChildrenStyle() const {
		return m_style;
	}
		/* RelationSet is called ONCE to find relation between tags */
	virtual void RelationSet(HTMLFrame *view);

		/* geometry() is a hook to set all static attributes.
		   It is called ONCE, after relations between all tag are set */
	virtual void geometry(HTMLFrame *) { };

		/* dynamicGeometry is called before evry reformating.
		   It is called in a depth-first manner */
	virtual void dynamicGeometry();

	virtual void initPlacement();
	virtual void draw(HTMLFrame *, bool onlyIfChanged=false) = 0;
	virtual const char* toString() const = 0;
	DocElem *Next() const { return m_next; }
	DocElem *Prev() const { return m_prev; }
	DocElem *Parent() const { return m_up; }
	DocElem *Predecessor() const { return m_up?m_up:m_prev; }
	DocElem *Last() const { return m_last; }
	DocElem *Included() const { return m_included; }
	virtual DocElem *Hides() const { return NULL; }
	virtual void pushNext(DocElemStack *s) {
		if (m_next) s->push(m_next);
	}
	virtual void pushChildren(DocElemStack *s) {
		if (m_included) s->push(m_included);
	}
	virtual void pushHidden(DocElemStack *s) {
		if (Hides()) s->push(Hides());
	}
	virtual bool close(const Tag * /*symbol*/, DocElem * /*last*/) { return false; };
	virtual void ClosingParent(DocElem * /*openningTag*/) {
		/* This function may seem obscure but is called on closing tags
		   with openningTag being the matching openning tag.
		   The main action in this callback should be "m_up = openningTag"
		   but here we are in DocElem and not TagDocElem, so this operation
		   is not valid (it is valid in TagDocElem::ClosingParent()) */
		abort();
	};
	virtual bool update(int right, int top, int bottom);
	virtual void place();
	virtual void printPosition(const char *reason);
	virtual bool IsActive() { return false; }
	virtual bool Action(::Action action, UrlQuery * href);
	virtual const char *printSpecific() { return ""; }
};


#endif
