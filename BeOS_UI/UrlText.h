#ifndef URLTEXT_H
#define URLTEXT_H

#include <TextView.h>
#include <MessageFilter.h>
#include "StrPlus.h"

const int MAX_ENTRIES = 30;

#define URL_ENTERED 'EURL'

class UrlText;

class NotityWhenWindowMove : public BMessageFilter {
	UrlText *m_notify;
	BWindow *m_watch;
public:
	NotityWhenWindowMove(BWindow *watch, UrlText *text)
		: BMessageFilter(B_WINDOW_MOVED)
	{
		m_notify = text;
		m_watch = watch;
	}
	virtual filter_result Filter(BMessage *message, BHandler **target);
};

class UrlListView : public BView {
	typedef BView super;
	rgb_color bg;
	rgb_color selected_bg;
	rgb_color text_color;
	int selected;
	BTextView *m_target;
	StrRef m_entries[MAX_ENTRIES];
	int m_nbEntries;
public:
	UrlListView(BRect frame, const char *name) :BView(frame, name, B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE) {
		bg = ui_color(B_MENU_BACKGROUND_COLOR);
		selected_bg = ui_color(B_MENU_SELECTION_BACKGROUND_COLOR);
		text_color = ui_color(B_MENU_ITEM_TEXT_COLOR);
		SetViewColor(bg);
		selected = -1;
		m_target = NULL;
		m_nbEntries = 0;
	}
	void SetTarget(BTextView *text) {
		m_target = text;
	}
	const char * FirstEntry() const {
		if (selected==-1)
			return NULL;
		else
			return m_entries[selected].Str();
	}
	virtual int GetChoices(StrRef entries[], int maxEntries, const char *pattern) {
		char *t[] = {
			"http://www.be.com",
			"http://www.yahoo.com",
			"http://www.yahoo.com/shop",
			"http://groups.yahoo.com",
			"http://www.free.fr",
			"http://dot.com",
			NULL
		};
		int nb = 0;
		for (int p = 0; nb<maxEntries && t[p]; p++) {
			if (strstr(t[p],pattern)) {
				entries[nb].SetToConst(t[p]);
				nb++;
			}
		}
		return nb;
	}
	virtual void MessageReceived(BMessage *message) {
		//printf("--------- Message in UrlListView %4s\n", &message->what);
		super::MessageReceived(message);
	}
	void Draw(BRect update) {
		int i = 0;
		for (int p = 0; p<m_nbEntries && !m_entries[p].IsFree(); p++) {
			if (selected == i) {
				SetHighColor(selected_bg);
				FillRect(BRect(0,12*i,300,12*i+12));
				SetLowColor(selected_bg);
			} else {
				SetLowColor(bg);
			}
			SetHighColor(text_color);
			DrawString(m_entries[p].Str(), BPoint(5,12*i+10));
			i++;
		}
	}
	int  SetPattern(const char *p) {
		if (p==NULL) {
			m_nbEntries = 0;
			return 0;
		} else {
			m_nbEntries = History::history.Query(m_entries, MAX_ENTRIES, p);
			return m_nbEntries;
		}
	}
	virtual void MakeFocus(bool focused = true) {
		if (focused) {
			if (selected==-1)
				selected=0;
		} else {
			selected=-1;
		}
		super::MakeFocus(focused);
		Invalidate();
	}
	void MoveSelection(int offset) {
		if (selected == -1) {
			selected = offset>0 ? 0 : m_nbEntries-1;
		} else {
			selected = min(max(selected+offset, 0), m_nbEntries-1);
		}
		Invalidate();
	}
};

class DropDownListWindow : public BWindow {
public:
	DropDownListWindow(BRect frame, const char *name) :
		BWindow(frame,name,B_BORDERED_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,B_AVOID_FOCUS )
	{
	}
	virtual void MessageReceived(BMessage *message) {
		//printf("--------- Message in DropDownListWindow %4s\n", &message->what);
		BWindow::MessageReceived(message);
	}
};

class UrlText : public BTextView {
	typedef BTextView super;
	char url[1024];
	DropDownListWindow *popupWindow;
	UrlListView *listView;
public:
	UrlText(BRect r, BRect textr) : BTextView(r, "textview", textr, 0) {
		url[0]=0;
		listView = NULL;
		popupWindow = NULL;
	}
	~UrlText() {
		if (popupWindow && popupWindow->Lock()) {
			popupWindow->Quit();
		}
	}
	bool CanEndLine(int32 offset) { 
		return false;
	}
	void SetUrl(const char *url) {
		SetText(url);
		int len = TextLength();
		Select(len, len);
	}
	virtual void AttachedToWindow() {
		popupWindow = new DropDownListWindow(BRect(10,50,300,150), "popupWindow-url");
		listView = new UrlListView(popupWindow->Bounds(), "urlListView");
		popupWindow->AddChild(listView);
		NotityWhenWindowMove *n = new NotityWhenWindowMove(popupWindow, this);
		Window()->AddFilter(n);	

		BWindow *win = Window();
		if (popupWindow && win && win->Lock()) {
			if (popupWindow->AddToSubset(win)!=B_OK) {
				printf("XXX GLOUPS\n");
			}
			win->Unlock();

			popupWindow->Show();
			popupWindow->Hide();
		} else {
			printf("XXX could not open popupWindow window\n");
		}

		super::AttachedToWindow();
	}
	virtual void MessageReceived(BMessage *message) {
		//printf("--------- Message in UrlText %4s\n", &message->what);
		super::MessageReceived(message);
	}
	void WindowEvent() {
		BPoint p = ConvertToScreen(Bounds().LeftBottom());
		if (popupWindow->Lock()) {
			popupWindow->MoveTo(p);
			popupWindow->Unlock();
		}
	}
	virtual void KeyDown(const char* bytes, int32 numBytes) {
		bool displayMatches = false;
		if (numBytes==1) {
			switch (bytes[0]) {
			case B_UP_ARROW:
			case B_DOWN_ARROW:
				if (popupWindow->Lock()) {
					listView->MoveSelection(bytes[0]==B_UP_ARROW ? -1 : 1);
					popupWindow->Unlock();
				}
				return;
			case B_TAB: {
				const char *text = listView->FirstEntry();
				if (text) {
					SetUrl(text);
				}
				return;
			}
			case B_ESCAPE:
				// XXX TODO Restore original url
				if (popupWindow->Lock()) {
					while (!popupWindow->IsHidden()) {
						popupWindow->Hide();
					}
					popupWindow->Unlock();
				}
				return;
			case B_ENTER: {
				if (popupWindow->Lock()) {
					if (!popupWindow->IsHidden()) {
						const char *text = listView->FirstEntry();
						if (text) {
							SetUrl(text);
						}
					}
					popupWindow->Unlock();
				}
				popupWindow->Hide();
				int len = TextLength();
				Select(0, len);
				BWindow *win = Window();
				if (win) {
					win->PostMessage(URL_ENTERED);
				}
				return;
				}
			default: {			
				displayMatches = true;
				break;
				}
			}
		}
		super::KeyDown(bytes, numBytes);
		if (displayMatches)
				DisplayMatch();
	}
private:
	void DisplayMatch() {
		GetText(0, sizeof(url)-1, url);
		if (popupWindow->Lock()) {
			if (listView->SetPattern(url)>0) {
				listView->Invalidate();
				BPoint p = ConvertToScreen(Bounds().LeftBottom());
				if (popupWindow->Lock()) {
					popupWindow->MoveTo(p);
					while (popupWindow->IsHidden())
						popupWindow->Show();
					popupWindow->Unlock();
				}
			} else {
				while (!popupWindow->IsHidden())
						popupWindow->Hide();
			}
			popupWindow->Unlock();
		}
	}
};


filter_result NotityWhenWindowMove::Filter(BMessage *message, BHandler **target) {
	m_notify->WindowEvent();
	return B_DISPATCH_MESSAGE;
}

#endif
