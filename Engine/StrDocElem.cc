#include <UTF8.h>
#include "StrDocElem.h"
#include "Frame.h"

void StrDocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	if (onlyIfChanged) return;
	view->DrawString(x,y+h-3,w-5,str,m_style); // XXX I don't really like the "-3"
	printPosition("Str_draw");
}

void StrDocElem::geometry(HTMLFrame *view) {
	char *destBuf; 
	uint32 enc = view->SourceEncoding();
	int32 destLen = strlen(str);
	if (0 != enc) {
		char *srcBuf = strdup(str);
		int32 srcLen = destLen;
		destLen = srcLen*4+1;
		int32 state = 0;
		destBuf = (char *) malloc(destLen); // reserve twice of what we got in  
		memset(destBuf, 0, destLen);
		convert_to_utf8(enc, srcBuf, &srcLen, destBuf, &destLen, &state);
		FREE(srcBuf);
	} else destBuf = strdup(str);

	view->StringDim(destBuf, m_style,&w,&h);
	w+=2; // XXX Some spaces between words : bad
	fixedW = w;
	FREE(destBuf);
}
