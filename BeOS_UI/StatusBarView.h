#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <View.h>
#include "StrPlus.h"

class StatusBarView : public BView {
	char msg[512];
public:
	StatusBarView(BRect r, const char *name, uint32 resizeMask,
						uint32 flags) : BView(r, name, resizeMask, flags) {
		msg[0]='\0';
	}
	void SetText(const char *s) {
		if (!strnull(s) && strcmp(msg,s)) {
			strncpy(msg, s, sizeof(msg)-1);
			msg[sizeof(msg)-1] = '\0';
			Invalidate();
		}
	}
	void Draw(BRect updateRect);
};


#endif
