#include <UTF8.h>
#include "StrDocElem.h"
#include "Frame.h"
#include <stdio.h>

void StrDocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	if (onlyIfChanged) return;
	view->DrawString(x,y+h-1,w,str,m_style); // XXX I don't really like the "-1"
	printPosition("Str_draw");
}

void StrDocElem::geometry(HTMLFrame *view) {
	char * buf = view->Decode(str);
	free(str);
	str = buf;
	view->StringDim(buf, m_style,&w,&h);
	fixedW = w;
}
