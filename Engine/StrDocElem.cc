#include <UTF8.h>
#include "StrDocElem.h"
#include "Frame.h"
#include <stdio.h>

void StrDocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	if (onlyIfChanged) return;
	char * buf = view->Decode(str);
	view->DrawString(x,y+h-3,w-5,buf,m_style); // XXX I don't really like the "-3"
	FREE(buf);
	printPosition("Str_draw");
}

void StrDocElem::geometry(HTMLFrame *view) {
	char * buf = view->Decode((const char *)str);
	view->StringDim(buf, m_style,&w,&h);
	FREE(buf);
	w+=2; // XXX Some spaces between words : bad
	fixedW = w;
}
