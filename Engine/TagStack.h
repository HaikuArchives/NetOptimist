#ifndef TAGSTACK
#define TAGSTACK

#include "traces.h"

class Tag;

class TagStack {
	const int SIZE;
	Tag **stack;
	int sp;
public:
	TagStack() : SIZE(200) { sp=0; stack = new Tag*[SIZE]; }
	~TagStack() {
		while (sp>0)
			pop();
		delete[] stack;
	}
	int height() { return sp; }
	void push(const char *s) {
		assert(s[0]!='/');
		if (sp<SIZE-1) {
			stack[sp] = new Tag(s);
			sp++;
		} else {
			fprintf(stderr, "Error : cannot push %s : stack full\n", s);
			dump();
		}
	}
	Tag* ElementAt(int pos) {
		assert(sp>pos);
		return stack[pos];
	}
	Tag* head() {
		assert(sp>0);
		return stack[sp-1];
	}
	void pop() {
		assert(sp>0);
		trace(TAG_STACK) {
			fprintf(stdout, "Poping : %s\n", stack[sp-1]->toString());
		}
		delete stack[sp-1];
		sp--;
	}
	int contains(const char *t) {
		for(int i=sp-1; i>=0; i--) {
			if (!strcmp(t, stack[i]->toString()))
				return i;
		}
		return -1;
	}
	void dump() {
		for(int i=0; i<sp; i++) {
			printf(">%s",stack[i]->toString());
		}
		if (sp) printf("\n");
	}
};

#endif
