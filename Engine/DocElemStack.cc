#include <stdio.h>
#include "DocElem.h"
#include "DocElemStack.h"

void DocElemStack::push(DocElem *s) {
	assert(sp<SIZE-1);
	//printf("%4d Pushing DocElem %d(%s) next %d included %d\n", sp, s->id, s->toString(), s->Next()?s->Next()->id:-1, s->Included()?s->Included()->id:-1);
	stack[sp] = s;
	sp++;
}

DocElem* DocElemStack::head() {
	assert(sp>0);
	return stack[sp-1];
}

void DocElemStack::pop() {
	//printf("%4d Poping DocElem %d\n", sp, stack[sp-1]->id);
	assert(sp>0);
	sp--;
	stack[sp]=NULL;
}

