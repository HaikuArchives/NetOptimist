#include "DocWalker.h"

DocWalker::DocWalker(DocElem *start) :
	stack()
{
	if (start) {
		stack.push(start);
	}
}

DocElem * DocWalker::Next() {
	DocElem *next = NULL;
	if (stack.height() > 0) {
		next = stack.head();
		stack.pop();
	}
	return next;
}

void DocWalker::Feed(DocElem *start) {
	start->pushNext(&stack);
	start->pushChildren(&stack);
}

void DocWalker::FeedHidden(DocElem *start) {
	start->pushHidden(&stack);
}
