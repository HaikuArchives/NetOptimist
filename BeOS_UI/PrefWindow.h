#ifndef __PrefWindow_h
#define __PrefWindow_h

#include <Window.h>

class Pref;

class PrefWindow : public BWindow {
public:
	PrefWindow();
	void MessageReceived(BMessage *message);
	Pref * Settings() { return m_pref; }
private:
	Pref *m_pref;
};

#endif	//	__PrefWindow_h
