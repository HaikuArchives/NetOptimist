#ifndef Constraint_H
#define Constraint_H

#include "traces.h"

class DocElem;
class TableDocElem;
class TR_DocElem;
class TD_DocElem;

class Constraint {
	int minX;
	int maxX;
	int nextX;
	int nextY;
	//int nextLineY;
	int lineH;
	int minW;
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
		//nextLineY = top;
		lineH = 0;
	}
	void NewLine(short indent, int vspace) {
		nextX = minX + indent;
		nextY += lineH + vspace;
		lineH = 0;
	}
	void NextPosition(int *x, int *y, int w, int h, short indent) {
		if (nextX>minX+indent && nextX+w-1>maxX)
			NewLine(indent, 0);

		*x = nextX;
		*y = nextY;
		nextX += w;
		if (h>lineH) lineH = h;
	}
	int Width() const {
		return maxX-minX;
	}
	int MaxX() const {
		return maxX;
	}
	int NextLineY() const {
		//return nextLineY;
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
