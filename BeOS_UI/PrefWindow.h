#ifndef __PrefWindow_h
#define __PrefWindow_h

#include <Window.h>

class PrefWindow : public BWindow {
public:
	PrefWindow();
	void MessageReceived(BMessage *message);
};

#endif	//	__PrefWindow_h
