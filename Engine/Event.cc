#include <stdio.h>
#include "Event.h"
#include "traces.h"

Events::Events() : SIZE(50) { 
	start = count = 0;
}

void Events::Add(int evt, bool noduplicate) {
	// XXX noduplicate is not handled
	assert(count<SIZE);
	assert(noduplicate == false);
	event[(start+count)%SIZE].code=evt;
	trace(DEBUG_EVENT) printf("Events::Add %4.4s\n", (char*)&evt);
	count++;
}

int Events::NextEvent() {
	if (count>0) {
		int code = event[start].code;
		start++;
		count--;
		if (start>=SIZE)
			start = 0;
		return code;
	} else {
		return 0;
	}
}
