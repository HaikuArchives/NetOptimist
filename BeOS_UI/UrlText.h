#ifndef URLTEXT_H
#define URLTEXT_H

#include <TextView.h>
#include "StrPlus.h"

const int MAX_ENTRIES = 30;

#define URL_ENTERED 'EURL'

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
		return m_entries[0].Str();
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
	void SetPattern(const char *p) {
		m_nbEntries = GetChoices(m_entries, MAX_ENTRIES, p);
	}
	virtual void KeyDown(const char* bytes, int32 numBytes) {
		if (numBytes==1) {
			switch(bytes[0]) {
			case B_DOWN_ARROW:
				selected++;
				Invalidate();
				break;
			case B_UP_ARROW:
				selected--;
				if (selected<0) selected=0;
				Invalidate();
				break;
			case B_ENTER:
				if (m_target && selected>=0) m_target->SetText(m_entries[selected].Str());
				Hide();
				break;
			case B_ESCAPE:
				selected = -1;
				if (m_target) m_target->MakeFocus();
				Invalidate();
			}
		}
		super::KeyDown(bytes, numBytes);
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
};

class DropDownListWindow : public BWindow {
public:
	DropDownListWindow(BRect frame, const char *name) :
		BWindow(frame,name,B_BORDERED_WINDOW_LOOK, /*B_NORMAL_WINDOW_FEEL*/B_FLOATING_SUBSET_WINDOW_FEEL,B_AVOID_FOCUS )
	{
	}
};

class UrlText : public BTextView {
	typedef BTextView super;
	char url[1024];
	DropDownListWindow *popup;
	UrlListView *listView;
public:
	UrlText(BRect r, BRect textr) : BTextView(r, "textview", textr, 0) {
		url[0]=0;
		listView = NULL;
		popup = NULL;
	}
	~UrlText() {
		printf("XXX ~UrlText() locking\n");
		if (popup && popup->Lock()) {
			printf("XXX ~UrlText() quitting\n");
			popup->Quit();
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
		popup = new DropDownListWindow(BRect(10,50,300,150), "popup-url");
		listView = new UrlListView(popup->Bounds(), "urlListView");
		popup->AddChild(listView);

		BWindow *win = Window();
		if (win && win->Lock()) {
			if (popup->AddToSubset(win)!=B_OK) {
				printf("XXX GLOUPS\n");
			}
			win->Unlock();

			popup->Show();
			popup->Hide();
		} else {
			printf("XXX could not open popup window\n");
		}

		//popup->SetTarget(this);
		//Window()->AddChild(popup);
		//popup->Hide();
		super::AttachedToWindow();
	}
	virtual void MessageReceived(BMessage *message) {
// NEXUS: was quite annoying, so i commented it
//		printf("got message\n");
		super::MessageReceived(message);
	}
	virtual void KeyDown(const char* bytes, int32 numBytes) {
		if (numBytes==1 && bytes[0]==B_DOWN_ARROW) {
			printf("got down key\n");
			//popup->MakeFocus();
		} else if (numBytes==1 && bytes[0]==B_TAB) {
			const char *text = listView->FirstEntry();
			if (text) {
				SetUrl(text);
			}
		} else {
			super::KeyDown(bytes, numBytes);
		}
	}
protected:
	void InsertText(const char *text, int32 length, int32 offset, const text_run_array *array) {
		if (text[0]=='\033') {
			// XXX TODO Restore original url
			if (popup->Lock()) {
				while (!popup->IsHidden()) {
					popup->Hide();
				}
				popup->Unlock();
			}
		} else if (text[0]!='\n') {
			printf("inserting %s (%d)\n", text, text[0]);
			BTextView::InsertText(text, length, offset, array);
#if 0 // XXX I must improve this before enabling 
			BPoint p = ConvertToScreen(Bounds().LeftBottom());
			if (popup->Lock()) {
				popup->MoveTo(p);
				while (popup->IsHidden())
					popup->Show();
				popup->Unlock();
			}
#endif
			DisplayMatch();
		} else {
			popup->Hide();
			int len = TextLength();
			Select(0, len);
			printf("Action !!!\n");
			BWindow *win = Window();
			if (win && length==1) {
				win->PostMessage(URL_ENTERED);
			}
		}
	}
	void DeleteText(int32 start, int32 finish) {
		BTextView::DeleteText(start, finish);
#if 0 // XXX I must improve this before enabling 
		if (popup->Lock()) {
			while (popup->IsHidden())
				popup->Show();
			popup->Unlock();
		}
#endif
		DisplayMatch();
	}
private:
	void DisplayMatch() {
		GetText(0, sizeof(url)-1, url);
		printf("value = %s\n", url);
		if (listView->LockLooper()) {
			listView->SetPattern(url);
			listView->Invalidate();
			listView->UnlockLooper();
		}
	}
};

#endif
