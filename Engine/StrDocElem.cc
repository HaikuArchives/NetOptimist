#include <UTF8.h>
#include "StrDocElem.h"
#include "Frame.h"
#include <stdio.h>

// the caller should free up the memory referenced by the pointer returned
char * StrDocElem::Decode(const char * str, uint32 enc) {
	char *destBuf; 
	int32 destLen = strlen(str);
	if (0 != enc) {
		char *srcBuf = strdup(str);
		int32 srcLen = destLen;
		destLen = srcLen*4+1;
		int32 state = 0;
		destBuf = (char *) malloc(destLen); 
		memset(destBuf, 0, destLen);
		convert_to_utf8(enc, srcBuf, &srcLen, destBuf, &destLen, &state);
		FREE(srcBuf);
	} else destBuf = strdup(str);
	return destBuf;
}

void StrDocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	if (onlyIfChanged) return;
	char * buf = this->Decode(str, view->SourceEncoding());
	view->DrawString(x,y+h-3,w-5,buf,m_style); // XXX I don't really like the "-3"
	FREE(buf);
	printPosition("Str_draw");
}

void StrDocElem::geometry(HTMLFrame *view) {
	char * buf = this->Decode(str, view->SourceEncoding());
	view->StringDim(buf, m_style,&w,&h);
	FREE(buf);
	w+=2; // XXX Some spaces between words : bad
	fixedW = w;
}
