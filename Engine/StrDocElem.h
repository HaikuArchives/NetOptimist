#ifndef StrDocElem_H
#define StrDocElem_H

#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "DocElem.h"
#include "Frame.h"

class StrDocElem : public DocElem {
public:
	char *str;
	StrDocElem(const char *text) : DocElem() { str = strdup(text); }
	StrDocElem(const char *text, int len) : DocElem() {
		str = (char*)malloc(len+1);
		memcpy(str, text, len);
		str[len] = '\0';
	}
	virtual ~StrDocElem() { free(str); }
	virtual void draw(HTMLFrame *m_view, const BRect *, bool onlyIfChanged=false);
	virtual void geometry(HTMLFrame *view);
	virtual const char * toString() const { return "STR"; }
	virtual const char * printSpecific() { return str; }
};


#endif
