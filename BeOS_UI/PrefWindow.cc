#include <Application.h>
#include <TextControl.h>
#include <TabView.h>
#include <Box.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <CheckBox.h>
#include <Button.h>
#include <Font.h>
#include <stdlib.h>
#include <stdio.h>

#include "PrefWindow.h"
#include "UIMessages.h"
#include "Pref.h"

#define WND_X	100
#define WND_Y 100
#define WND_W 500
#define WND_H 400
#define WND_TITLE	"Preferences"

#define PRINT_RECT(r) \
	fprintf(stdout, "BRect (%f, %f):(%f, %f)\n", r.left, r.top, r.right, r.bottom);

#define LEFT_MARGIN 6
#define MARGIN 10
	 
// General Properies
class GeneralView: public BView {
public:
	GeneralView(BRect r);
};

GeneralView::GeneralView(BRect rect) : BView(rect, "general_tab_view", B_FOLLOW_ALL, 
	B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE) {
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect r = Bounds();
	r.bottom = r.top + 15;
	float divider = 165; //r.Width() / 2; // fixed divider
	BTextControl *text = new BTextControl(r, "home_page", "Home page:", Pref::Default.HomePage(), new BMessage(bmsgPrefGenHomePage)); 
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetDivider(divider);
	AddChild(text);

	r.OffsetBy(0,25);
	
	text = new BTextControl(r, "search_page", "Search page:", Pref::Default.SearchPage(), new BMessage(bmsgPrefGenSearchPage));
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetDivider(divider);
	AddChild(text);
	
	r.OffsetBy(0,25);
	
	text = new BTextControl(r, "download_directory", "Download directory:", Pref::Default.DownloadDirectory(), new BMessage(bmsgPrefGenDownloadDirectory));
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetDivider(divider);
	AddChild(text);

	r.OffsetBy(0,25);

	BPopUpMenu *menu = new BPopUpMenu("new_window_menu");
	BMenuItem *menu_item;
	menu->AddItem(menu_item = new BMenuItem("Clone current window", new BMessage(bmsgPrefGenCloneWindow)));

	if (NO_CLONE_CURRENT == Pref::Default.NewWindowAction())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Open home page", new BMessage(bmsgPrefGenOpenHome)));
	if (NO_OPEN_HOME == Pref::Default.NewWindowAction())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Open blank page", new BMessage(bmsgPrefGenOpenBlank)));
	if (NO_OPEN_BLANK == Pref::Default.NewWindowAction())
		menu_item->SetMarked(true);
	
	BMenuField *menu_field = new BMenuField(r, "new_window", "New windows:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(divider+2);
	AddChild(menu_field);

	r.OffsetBy(0,27);

	menu = new BPopUpMenu("cookie_menu");
	menu->AddItem(menu_item = new BMenuItem("Accept", new BMessage(bmsgPrefGenCookieAccept)));

	if (NO_ACCEPT_COOKIE == Pref::Default.CookieAction())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Reject", new BMessage(bmsgPrefGenCookieReject)));
	if (NO_REJECT_COOKIE == Pref::Default.CookieAction())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Ask", new BMessage(bmsgPrefGenCookieAsk)));
	if (NO_ASK_COOKIE == Pref::Default.CookieAction())
		menu_item->SetMarked(true);
	
	menu_field = new BMenuField(r, "cookie", "When a new cookie is received:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(divider+2);
	AddChild(menu_field);
	
	r.OffsetBy(0,39);
	r.bottom = r.top+1;
	BBox *box = new BBox(r);
	AddChild(box);

	r.OffsetBy(0,15);
	BCheckBox *chkbox = new BCheckBox(r, 
		"launch_downloads", 
		"Automatically launch files after they have finished downloading.", 
		new BMessage(bmsgPrefGenAutoLaunch));
	chkbox->SetValue((int)Pref::Default.LaunchDownloaded());
	AddChild(chkbox);

	r.OffsetBy(0,25);
	r.right -= 70; 
	char *days = new char[16]; // 16 digits should be enough
	sprintf(days, "%d", Pref::Default.DaysInGo());
	text = new BTextControl(r, "days_in_go", "Number of days to keep links in the Go menu:", days, new BMessage(bmsgPrefGenDaysInGo));
	text->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);
	text->SetDivider(230);
	AddChild(text);
}


// Display Properties
class DisplayView: public BView {
public:
	DisplayView(BRect r);
};

DisplayView::DisplayView(BRect rect) : BView(rect, "display_tab_view", B_FOLLOW_ALL, B_NAVIGABLE) {
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect r = Bounds();
	r.bottom = r.top + 15;
	float divider = 95; 

	BPopUpMenu *menu = new BPopUpMenu("encoding_menu");
	BMenuItem *menu_item;
	menu->AddItem(menu_item = new BMenuItem("Western", new BMessage(bmsgPrefDispEncWestern)));

	// FIXME: 	
	menu_item->SetMarked(true);

	menu->AddItem(menu_item = new BMenuItem("Unicode", new BMessage(bmsgPrefDispEncUnicode)));
	menu->AddItem(menu_item = new BMenuItem("Japanese", new BMessage(bmsgPrefDispEncJapanese)));
	menu->AddItem(menu_item = new BMenuItem("Greek", new BMessage(bmsgPrefDispEncGreek)));
	menu->AddItem(menu_item = new BMenuItem("Cyrillic", new BMessage(bmsgPrefDispEncCyrillic)));
	menu->AddItem(menu_item = new BMenuItem("Central European", new BMessage(bmsgPrefDispEncCE)));
	
	BMenuField *menu_field = new BMenuField(r, "for_encoding", "For the encoding:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(divider+2);
	AddChild(menu_field);

	r.OffsetBy(0,25);
	
	menu = new BPopUpMenu("proportional_menu");
	BMessage *msg;

	int32 numFamilies = count_font_families();
	for ( int32 i = 0; i < numFamilies; i++ ) {
		font_family family;
		uint32 flags;
		if ( get_font_family(i, &family, &flags) == B_OK ) {
			menu->AddItem(menu_item = new BMenuItem(family, msg = new BMessage(bmsgPrefDispPropFont)));
			msg->AddString("family", family);

			// FIXME: select correct one, now first is selected
			if (0 ==i)
				menu_item->SetMarked(true);
		}
	}
	menu_field = new BMenuField(r, "proportional_font", "Proportional font:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(divider+2);
	AddChild(menu_field);

	BRect r1 = r;
	r1.OffsetBy(200,0);
	r1.right = r1.left+80;
	menu = new BPopUpMenu("prop_size_menu");
	menu->AddItem(menu_item = new BMenuItem("7", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 7);

	// FIXME: 	
	menu_item->SetMarked(true);

	menu->AddItem(menu_item = new BMenuItem("9", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 9);
	menu->AddItem(menu_item = new BMenuItem("10", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 10);
	menu->AddItem(menu_item = new BMenuItem("12", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 12);
	menu->AddItem(menu_item = new BMenuItem("14", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 14);
	menu->AddItem(menu_item = new BMenuItem("18", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 18);
	menu->AddItem(menu_item = new BMenuItem("24", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 24);

	menu_field = new BMenuField(r1, "prop_font_size", "Size:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(34);
	AddChild(menu_field);
	
	r1.OffsetBy(81,0);
	menu = new BPopUpMenu("prop_msize_menu");
	menu->AddItem(menu_item = new BMenuItem("7", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 7);

	// FIXME: 	
	menu_item->SetMarked(true);

	menu->AddItem(menu_item = new BMenuItem("9", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 9);
	menu->AddItem(menu_item = new BMenuItem("10", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 10);
	menu->AddItem(menu_item = new BMenuItem("12", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 12);
	menu->AddItem(menu_item = new BMenuItem("14", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 14);
	menu->AddItem(menu_item = new BMenuItem("18", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 18);
	menu->AddItem(menu_item = new BMenuItem("24", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 24);

	menu_field = new BMenuField(r1, "prop_mfont_size", "Min. size:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(be_plain_font->StringWidth("Min. size:")/*50*/);
	AddChild(menu_field);

	r.OffsetBy(0,25);
	
	menu = new BPopUpMenu("fixed_menu");

	for ( int32 i = 0; i < numFamilies; i++ ) {
		font_family family;
		uint32 flags;
		if ( get_font_family(i, &family, &flags) == B_OK ) {
			menu->AddItem(menu_item = new BMenuItem(family, msg = new BMessage(bmsgPrefDispFixedFont)));
			msg->AddString("family", family);

			// FIXME: select correct one, now first is selected
			if (0 ==i)
				menu_item->SetMarked(true);
		}
	}

	menu_field = new BMenuField(r, "fixed_font", "Fixed font:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(divider+2);
	AddChild(menu_field);

	r1 = r;
	r1.OffsetBy(200,0);
	r1.right = r1.left+80;
	menu = new BPopUpMenu("fixed_size_menu");
	menu->AddItem(menu_item = new BMenuItem("7", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 7);

	// FIXME: 	
	menu_item->SetMarked(true);

	menu->AddItem(menu_item = new BMenuItem("9", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 9);
	menu->AddItem(menu_item = new BMenuItem("10", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 10);
	menu->AddItem(menu_item = new BMenuItem("12", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 12);
	menu->AddItem(menu_item = new BMenuItem("14", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 14);
	menu->AddItem(menu_item = new BMenuItem("18", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 18);
	menu->AddItem(menu_item = new BMenuItem("24", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 24);

	menu_field = new BMenuField(r1, "fixed_font_size", "Size:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(34);
	AddChild(menu_field);
	
	r1.OffsetBy(81,0);
	menu = new BPopUpMenu("fixed_msize_menu");
	menu->AddItem(menu_item = new BMenuItem("7", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 7);

	// FIXME: 	
	menu_item->SetMarked(true);

	menu->AddItem(menu_item = new BMenuItem("9", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 9);
	menu->AddItem(menu_item = new BMenuItem("10", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 10);
	menu->AddItem(menu_item = new BMenuItem("12", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 12);
	menu->AddItem(menu_item = new BMenuItem("14", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 14);
	menu->AddItem(menu_item = new BMenuItem("18", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 18);
	menu->AddItem(menu_item = new BMenuItem("24", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 24);

	menu_field = new BMenuField(r1, "fixed_mfont_size", "Min. size:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(be_plain_font->StringWidth("Min. size:")/*50*/);
	AddChild(menu_field);

	r.OffsetBy(0, 30);
	r.bottom = r.top+1;
	BBox *box = new BBox(r);
	AddChild(box);
	
	r.OffsetBy(0,25);
	BCheckBox *chkbox = new BCheckBox(r, 
		"show_images", 
		"Show images", 
		new BMessage(bmsgPrefDispShowImages));
	chkbox->SetValue((int)Pref::Default.ShowImages());
	AddChild(chkbox);

	r.OffsetBy(0,25);
	chkbox = new BCheckBox(r, 
		"show_bgimages", 
		"Show background images", 
		new BMessage(bmsgPrefDispShowBgImages));
	chkbox->SetValue((int)Pref::Default.ShowBgImages());
	AddChild(chkbox);

	r.OffsetBy(0,25);
	chkbox = new BCheckBox(r, 
		"show_animations", 
		"Show animations", 
		new BMessage(bmsgPrefDispShowAnimations));
	chkbox->SetValue((int)Pref::Default.ShowAnimations());
	AddChild(chkbox);

	r.OffsetBy(0,25);
	chkbox = new BCheckBox(r, 
		"underline_links", 
		"Underline links", 
		new BMessage(bmsgPrefDispUnderlineLinks));
	chkbox->SetValue((int)Pref::Default.UnderlineLinks());
	AddChild(chkbox);

	r.OffsetBy(0,25);
	chkbox = new BCheckBox(r, 
		"haiku_errors", 
		"Haiku error messages", 
		new BMessage(bmsgPrefDispHaikuErrors));
	chkbox->SetValue((int)Pref::Default.HaikuErrors());
	AddChild(chkbox);
}


// Connections Properties 
class ConnectionsView: public BView {
public:
	ConnectionsView(BRect r);
};

ConnectionsView::ConnectionsView(BRect rect) : BView(rect, "connections_tab_view", B_FOLLOW_ALL, B_NAVIGABLE) {
//	BRect r = Bounds();
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

// Cache Properties
class CacheView: public BView {
public:
	CacheView(BRect r);
};

CacheView::CacheView(BRect rect) : BView(rect, "cache_tab_view", B_FOLLOW_ALL, B_NAVIGABLE) {
//	BRect r = Bounds();
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

// Security Properties
class SecurityView: public BView {
public:
	SecurityView(BRect r);
};

SecurityView::SecurityView(BRect rect) : BView(rect, "security_tab_view", B_FOLLOW_ALL, B_NAVIGABLE) {
//	BRect r = Bounds();
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}


// Narrower tab class
class NarrowTab : public BTab {
public:
	NarrowTab();
	void DrawFocusMark(BView *, BRect);
};

NarrowTab::NarrowTab() {
}

void NarrowTab::DrawFocusMark(BView * view, BRect r) {
	PRINT_RECT(r);
	BTab::DrawFocusMark(view, r);
}

// Tab View
class TabView: public BTabView {
public:
	TabView(BRect, char *);
	BRect TabFrame(int32 tab_index);
	BRect DrawTabs();
	void MouseDown(BPoint point);
	virtual void SetFocusTab(int32, bool);
private:
};


/////////////
TabView::TabView(BRect r, char *name) : BTabView(r, name) {
}


/////////////
BRect TabView::TabFrame(int32 tab_index) {
	BRect r;
	r.top = 0;
	r.bottom += TabHeight();
	float prevx = LEFT_MARGIN;
	for (int32 i=0;i<tab_index+1;i++) {
		BTab *tab = TabAt(i);
		float width = be_plain_font->StringWidth(tab->Label());
		width += MARGIN*2; 
		r.left = prevx;
		r.right = r.left+width;
		prevx = r.right;
	}
	return r;
}


/////////////
BRect TabView::DrawTabs() {
	BRect r;
	BRect selectedRect;
	float prevx = LEFT_MARGIN;
	r.top = 0;
	r.bottom += TabHeight();
	
	for (int32 i=0;i<CountTabs();i++) {
		BTab *tab = TabAt(i);
		float width = be_plain_font->StringWidth(tab->Label());
		width += MARGIN*2; //+10 from each side
		r.left = prevx;
		r.right = r.left+width;
		prevx = r.right;
		tab_position pos = B_TAB_ANY;
		if (tab->IsSelected()) {
			pos = B_TAB_FRONT;
			selectedRect = r;
		}
		else if (0 == i)
			pos = B_TAB_FIRST;
		tab->DrawLabel(this, r);
		tab->DrawTab(this, r, pos, true);
	}
	return selectedRect;
}


/////////////
void TabView::MouseDown(BPoint point) {
	int32 count = CountTabs();
	BRect rect = Bounds();
	rect.bottom = rect.top +TabHeight();

	if(rect.Contains(point)) {
		BRect r;
		float prevx = LEFT_MARGIN;
		r.top = 0;
		r.bottom += TabHeight();
		for (int32 i=0;i<count;i++) {
			BTab *tab = TabAt(i);
			float width = be_plain_font->StringWidth(tab->Label());
			width += MARGIN*2; //+10 from each side
			r.left = prevx;
			r.right = r.left+width;
			prevx = r.right;
			if (r.Contains(point) && i != Selection())
				Select(i);
		}
	} else
		BTabView::MouseDown(point);
}


/////////////
void TabView::SetFocusTab(int32 tab_index, bool focus) {
	BTab *tab = TabAt(tab_index);
	BRect r = TabFrame(tab_index);
	tab->MakeFocus(focus);

	tab_position pos = B_TAB_ANY;
	if (tab->IsSelected())
		pos = B_TAB_FRONT;
	else if (0 == tab_index)
		pos = B_TAB_FIRST;
	tab->DrawTab(this, r, pos, true);

//	tab->DrawFocusMark(this, r);
  }

// Preferences Window

// Constructor
PrefWindow::PrefWindow() : BWindow(BRect(WND_X, WND_Y, WND_W, WND_H), 
	WND_TITLE, B_TITLED_WINDOW, B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_ASYNCHRONOUS_CONTROLS) {
	BRect r = Bounds();
	r.InsetBy(-1,-1);
	BBox *b = new BBox(r);
	r.left += 2;
	r.top += 13;
	r.bottom -= 45;
	TabView *tab_view = new TabView(r, "tab_view");
	tab_view->SetTabWidth(B_WIDTH_AS_USUAL);
	r = tab_view->ChildAt(0)->Bounds(); // get bounds of internal view
	r.InsetBy(13,9);
	NarrowTab *tab = new NarrowTab();
	tab_view->AddTab(new GeneralView(r), tab);
	tab->SetLabel("General");
	tab = new NarrowTab();
	tab_view->AddTab(new DisplayView(r), tab);
	tab->SetLabel("Display");	
	tab = new NarrowTab();
	tab_view->AddTab(new ConnectionsView(r), tab);
	tab->SetLabel("Connections");	
	tab = new NarrowTab();
	tab_view->AddTab(new CacheView(r), tab);
	tab->SetLabel("Cache");	
	tab = new NarrowTab();
	tab_view->AddTab(new SecurityView(r), tab);
	tab->SetLabel("Security");	
	b->AddChild(tab_view);

	// draw buttons now
	r = b->Bounds();
	r.top = r.bottom - 36; 
	r.bottom = r.top + 20;
	r.left = 231;
	r.right = r.left + 75;
	BButton *but = new BButton(r, "cancel_button", "Cancel", new BMessage(B_CANCEL));
	b->AddChild(but);
	r.OffsetBy(75+10, 0);
	but = new BButton(r, "ok_button", "OK", new BMessage(B_OK));
	but->MakeDefault(true);
	b->AddChild(but);
	AddChild(b);
	
}


// Message handler
void PrefWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case bmsgPrefGenHomePage: {
			BTextControl *text;
			if (B_OK == msg->FindPointer("source", (void **) &text))
				Pref::Default.SetHomePage(text->Text());
			break;
		}
		case bmsgPrefGenSearchPage: {
			BTextControl *text;
			if (B_OK == msg->FindPointer("source", (void **) &text))
				Pref::Default.SetSearchPage(text->Text());
			break;
		}
		case bmsgPrefGenDownloadDirectory: {
			BTextControl *text;
			if (B_OK == msg->FindPointer("source", (void **) &text))
				Pref::Default.SetDownloadDirectory(text->Text());
			break;
		}
		case bmsgPrefGenCloneWindow:
			Pref::Default.SetNewWindowAction(NO_CLONE_CURRENT);
			break;
		case bmsgPrefGenOpenHome:
			Pref::Default.SetNewWindowAction(NO_OPEN_HOME);
			break;
		case bmsgPrefGenOpenBlank:
			Pref::Default.SetNewWindowAction(NO_OPEN_BLANK);
			break;
		case bmsgPrefGenCookieAccept:
			Pref::Default.SetCookieAction(NO_ACCEPT_COOKIE);
			break;
		case bmsgPrefGenCookieReject	:
			Pref::Default.SetCookieAction(NO_REJECT_COOKIE);
			break;
		case bmsgPrefGenCookieAsk:
			Pref::Default.SetCookieAction(NO_ASK_COOKIE);
			break;
		case bmsgPrefGenAutoLaunch: {
			BCheckBox *chkbox;
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				Pref::Default.SetLaunchDownloaded((bool)chkbox->Value());
			break;
		}
		case bmsgPrefGenDaysInGo: {
			BTextControl *text;
			if (B_OK == msg->FindPointer("source", (void **) &text))
				Pref::Default.SetDaysInGo(atoi(text->Text()));
			break;
		}
		
		case B_OK:
			Pref::Default.Save();
		case B_CANCEL:
			Quit();
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}
