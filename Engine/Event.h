#ifndef EVENT_H
#define EVENT_H

#include <assert.h>
#include "platform.h"

class EventProcessor;
class BLooper;

class Events {
	const int SIZE;
	struct {
		int code;
//		BLooper *dest;
	} event[50];
	int start, count;
public:
	Events();
	void Add(int evt, bool noduplicate = false);
	int NextEvent();
};

class EventProcessor {
protected:
	Events m_events;
public:
	void Msg(int evt) {
		m_events.Add(evt);
	}
	void ProcessAll() {
		int code;
		while ( (code=m_events.NextEvent()) != 0)
			Process(code);
	}
	virtual void Process(int msg) =0;
};

#endif
