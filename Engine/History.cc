
#include <stdio.h>
#include "StrPlus.h"
#include "History.h"

History History::history;

History::History() {
	cur = last = NULL;
}

History::~History() {
	HistLine *p = last;
	while(p) {
		last = p->prev;
		p->url.Free();
		delete(p);
		p = last;
	}
}

void History::Add(const StrRef *url) {
	HistLine *p = new HistLine;
	p->url.SetToRef(url);
	p->title.Free();
	p->nbVisits = 1;
	p->prev = last;
	p->next = NULL;
	if (last) {
		last->next = p;
	}

	cur = last = p;
}

StrRef *History::Back() {
	if (cur && cur->prev) {
		cur = cur->prev;
		return &cur->url;
	} else {
		return NULL;
	}
}

StrRef *History::Forward() {
	if (cur && cur->next) {
		cur = cur->next;
		return &cur->url;
	} else {
		return NULL;
	}
}


bool History::HasBack() {
	return (cur && cur->prev);
}

bool History::HasForward() {
	return (cur && cur->next);
}

int History::Query(StrRef entries[], int maxEntries, const char *url) {
	//if (strlen(url)<5) return;
	int nb = 0;
	HistLine *p = last;
	while (p && nb < maxEntries) {
		if (strstr(p->url.Str(), url)) {
			entries[nb].SetToRef(&p->url);
			nb++;
		}
		p = p->prev;
	}
	return nb;
}
