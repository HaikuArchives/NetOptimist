#ifndef DocElemStack_H
#define DocElemStack_H

#include <assert.h>

class DocElem;

static const int SIZE = 200;

class DocElemStack {
	DocElem *stack[SIZE];
	int sp;
public:
	DocElemStack() { sp=0; }
	~DocElemStack() { }
	int height() { return sp; }
	void push(DocElem *s);
	DocElem* head();
	void pop();
};

#endif
