#include "DocElem.h"
#include "Constraint.h"
#include "Style.h"
#include "Frame.h"
#include "traces.h"
#include "UrlQuery.h"

int DocElem::idGen=1;

void DocElem::printPosition(const char *reason) {
        trace(DEBUG_RENDER) printf("%10s:%5d %s\t^%4d >%4d v%4d x=%5d y=%5d w=%3d h=%3d S%s %d|%d.%d>%d_%d\t%s>\n",
		reason,
		id,
		toString(),
                Parent()?Parent()->id:0,
                Next()?Next()->id:0,
                m_included?m_included->id:0,
                x,y,w,h,
                m_style->toString(),
                constraint->creator->id,
                constraint->minX,
                constraint->nextX,
                constraint->maxX,
                constraint->NextLineY(),
		printSpecific());
}

void DocElem::initPlacement() {
	if (constraint->creator==this)
		constraint->Init(0,0,300);	// XXX 300 == not significant
	if (includedConstraint->creator==this)
		includedConstraint->Init(0,0,300);	// XXX 300 == not significant
}

void DocElem::place() {
	constraint->NextPosition(&x, &y, w, h, m_style->Indent(), m_style->Align());

	trace(DEBUG_RENDER)
		printPosition("place");
}

void DocElem::RelationSet(HTMLFrame * view) {
	if (m_up) {
		m_style = m_up->ChildrenStyle();
		if (m_up->includedConstraint)
			constraint = m_up->includedConstraint;
	} else if (m_prev) {
		if (m_prev->m_style)
			m_style = m_prev->m_style;
		if (m_prev->constraint)
			constraint = m_prev->constraint;
	}
	if (!m_style) m_style = StyleMgr::DefaultStyleMgr.DefaultStyle();
	if (!constraint) {
		// We should only get here for the root document element.
		if (m_prev || m_up) {
			fprintf(stderr, "Warning : no constraint from parent for non-root DocElem id=%d\n", id);
		}
		// This is the root DocElem.
		constraint = new Constraint(this, view->DocWidth()); /* XXX this is still needed for the root document element */
	}
	if (!includedConstraint)
		includedConstraint = constraint;
	geometry(view);
}

void DocElem::dynamicGeometry(HTMLFrame *) {
	if (includedConstraint && includedConstraint != constraint) {
		fixedW = max(fixedW, includedConstraint->minW);
	}
	if (constraint)
		constraint->RegisterWidth(w);
}

DocElem::~DocElem() {
	if (constraint && constraint->creator==this)
		delete constraint;
	if (includedConstraint != constraint && includedConstraint && includedConstraint->creator==this)
		delete includedConstraint;

	// First, delete our style if we are creator, then do the same for ChildrenStyle()
	Style *s;
	s = const_cast<Style *>(m_style);
	if (s && s->OwnerId()==id)
		delete s;
	m_style = NULL;	// This is necessary because ChildrenStyle() returns style by default
	s = const_cast<Style *>(ChildrenStyle());
	if (s && s->OwnerId()==id)
		delete s;
}

bool DocElem::update(int right, int top, int bottom) {
	if (includedConstraint && includedConstraint->creator==this) {
		if (includedConstraint->maxX<right) {
			fprintf(stdout, "update id=%d significant right %d was %d\n",id, right,includedConstraint->maxX);
			includedConstraint->maxX = max(includedConstraint->maxX, right);
			printPosition("update");
			return true;
		}
	}
	return false;
}

bool DocElem::Action(::Action action, UrlQuery * query) {
	query->m_name = NULL;
	if (action == MOUSE_OVER) { return false; }
	fprintf(stderr, "WARNING : doc element %d is marked as active but no action\n", id);
	return false;
}
