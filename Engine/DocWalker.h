#ifndef DocWalker_H
#define DocWalker_H

#include "DocElem.h"
#include "DocElemStack.h"

class DocWalker {
	DocElemStack stack;
public:
	DocWalker(DocElem *start);
	DocElem *Next();
	void Feed(DocElem *start);
	void FeedHidden(DocElem *start);
};

#endif
