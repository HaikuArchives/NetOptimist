#include <UTF8.h>
#include "StrDocElem.h"
#include "Frame.h"
#include <stdio.h>

void StrDocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	if (onlyIfChanged) return;
	view->DrawString(x,y+h-3,w-5,str,m_style); // XXX I don't really like the "-3"
	printPosition("Str_draw");
}

void StrDocElem::geometry(HTMLFrame *view) {
	char * buf = view->Decode(str);
	free(str);
	str = buf;
	view->StringDim(buf, m_style,&w,&h);
	w+=2; // XXX Some spaces between words : bad
	fixedW = w;
}
