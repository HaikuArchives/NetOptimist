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
	virtual ~StrDocElem() { free(str); }
	virtual void draw(HTMLFrame *m_view, bool onlyIfChanged=false);
	virtual void geometry(HTMLFrame *view);
	virtual const char * toString() const { return "STR"; }
	virtual const char * printSpecific() { return str; }
private:
	char * Decode(const char * str, uint32 encoding);
};


#endif
