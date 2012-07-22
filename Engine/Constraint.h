#ifndef Constraint_H
#define Constraint_H

#include <stdio.h>
#include "traces.h"

#include "Alignment.h"

class DocElem;
class TableDocElem;
class TR_DocElem;
class TD_DocElem;

class Constraint {
	int minX;
	int maxX;
	int nextX;
	int nextY;
	int lineH;
	int minW;
	int m_nbFloat;
	struct Obj {
		int w;
		int h;
		Alignment al;
	} m_float[5];
public:
	friend class TD_DocElem;
	friend class TableDocElem;
	friend class DocElem;
	DocElem *creator;
	Constraint(DocElem *owner, int w=0) : minX(0), maxX(w), nextX(0), nextY(0),
					lineH(0), creator(owner) {
		minW = 0;
	}
	void Init(int top, int left, int right) {
		minX = left;
		maxX = right;
		nextX = left;
		nextY = top;
		lineH = 0;
		m_nbFloat = 0;
	}
	void NewLine(short marginLeft, int marginTop) {
		if(marginTop<0) { fprintf(stderr, "XXX warning vspace<0\n"); }
		nextX = minX + marginLeft;
		nextY += lineH + marginTop;
		lineH = 0;
		if (m_nbFloat>0 && m_float[m_nbFloat-1].h<nextY) {
			// XXX should not remove the list, but shift elements;
			m_nbFloat = 0;
		}
		for (int i=0; i<m_nbFloat; i++) {
			if (m_float[i].al==al_left && m_float[i].h>=nextY) {
				nextX = max(m_float[i].w, nextX);
			}
		}
	}
	void NextPosition(int *x, int *y, int w, int h, short indent, Alignment align) {
		int i;
		if (m_nbFloat==5) align = al_bottom;	// XXX this is bad...
		switch (align) {
		case al_center: // XXX not supported
		case al_top: // XXX not supported
		case al_bottom:
			if (nextX>minX+indent && nextX+w-1>maxX)
				NewLine(indent, 0);
			for (i=0; i<m_nbFloat; i++) {
				if (m_float[i].al==al_right && nextX+w-1>m_float[i].w && nextY<=m_float[i].h) {
					if (nextX>minX+indent)
						NewLine(indent, 0);
					else
						NewLine(indent, m_float[i].h-nextY); // XXX not perfect, should test for all floating objects
				}
			}

			*x = nextX;
			*y = nextY;
			nextX += w;
			if (h>lineH) lineH = h;
			break;
		case al_right:
			*x = maxX - w;
			*y = nextY;
			m_float[m_nbFloat].w = *x;
			m_float[m_nbFloat].h = *y+h;
			m_float[m_nbFloat].al = align;
			m_nbFloat++;
			break;
		case al_left:
			*x = nextX;
			*y = nextY;
			nextX += w;
			m_float[m_nbFloat].w = *x+w;
			m_float[m_nbFloat].h = *y+h;
			m_float[m_nbFloat].al = align;
			m_nbFloat++;
			break;
		default:
			abort();
		}
	}
	int Width() const {
		return maxX-minX;
	}
	int MaxX() const {
		return maxX;
	}
	int NextLineY() const {
		return nextY + lineH;
	}
	void RegisterWidth(int item_minW) {
		if (item_minW>minW)
			minW = item_minW;
	}
	int MinW() const {
		return minW;
	}
	void Check() {
		trace(DEBUG_RENDER) if (Width()<MinW())
			fprintf(stderr, "Warning : constraint %s(%d) too small %d<%d!\n", creator->toString(), creator->id, Width(),MinW());
	}
};

#endif
