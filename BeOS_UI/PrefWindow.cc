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

// Constants below used for messaging inside the window
#define CTL_HTTP_PROXY_NAME	"http_proxy_name"
#define CTL_HTTP_PROXY_PORT	"http_proxy_port"
#define CTL_FTP_PROXY_NAME		"ftp_proxy_name"
#define CTL_FTP_PROXY_PORT		"ftp_proxy_port"
	 

// FocusButton is an utility class, which helps us to get focus changed when button is pressed 
// (it is not like this by default)
// It enables us to get a modification message from BTextControl when the OK button is 
// pressed or clicked
class FocusButton: public BButton {
public:
	FocusButton(BRect r, const char *name, const char *label, BMessage *msg, uint32 resizingMode, uint32 flags);
	virtual void MouseDown(BPoint point);
	virtual void KeyDown(const char *bytes, int32 numBytes);
};
FocusButton::FocusButton(BRect r, const char *name, const char *label, BMessage *msg, uint32 resizingMode = B_FOLLOW_LEFT|B_FOLLOW_TOP, uint32 flags = B_WILL_DRAW|B_NAVIGABLE) :
	BButton(r, name, label, msg, resizingMode, flags) {
}
void FocusButton::MouseDown(BPoint point) {
	MakeFocus(true);
	BButton::MouseDown(point);
}
void FocusButton::KeyDown(const char *bytes, int32 numBytes) {
	MakeFocus(true);
	BButton::KeyDown(bytes, numBytes);
}


// General Properies
class GeneralView: public BView {
public:
	GeneralView(BRect r, Pref *p);
private:
	Pref *pref; // shouldn't be deleted
};

GeneralView::GeneralView(BRect rect, Pref *p) : BView(rect, "general_tab_view", B_FOLLOW_ALL, 
	B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE) {
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	pref = p;	
	BRect r = Bounds();
	r.bottom = r.top + 15;
	float divider = 165; //r.Width() / 2; // fixed divider
	BTextControl *text = new BTextControl(r, "home_page", "Home page:", pref->HomePage(), new BMessage(bmsgPrefGenHomePage)); 
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetDivider(divider);
	AddChild(text);

	r.OffsetBy(0,25);
	
	text = new BTextControl(r, "search_page", "Search page:", pref->SearchPage(), new BMessage(bmsgPrefGenSearchPage));
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetDivider(divider);
	AddChild(text);
	
	r.OffsetBy(0,25);
	
	text = new BTextControl(r, "download_directory", "Download directory:", pref->DownloadDirectory(), new BMessage(bmsgPrefGenDownloadDirectory));
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetDivider(divider);
	AddChild(text);

	r.OffsetBy(0,25);

	BPopUpMenu *menu = new BPopUpMenu("new_window_menu");
	BMenuItem *menu_item;
	menu->AddItem(menu_item = new BMenuItem("Clone current window", new BMessage(bmsgPrefGenCloneWindow)));

	if (NO_CLONE_CURRENT == pref->NewWindowAction())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Open home page", new BMessage(bmsgPrefGenOpenHome)));
	if (NO_OPEN_HOME == pref->NewWindowAction())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Open blank page", new BMessage(bmsgPrefGenOpenBlank)));
	if (NO_OPEN_BLANK == pref->NewWindowAction())
		menu_item->SetMarked(true);
	
	BMenuField *menu_field = new BMenuField(r, "new_window", "New windows:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(divider+2);
	AddChild(menu_field);

	r.OffsetBy(0,27);

	menu = new BPopUpMenu("cookie_menu");
	menu->AddItem(menu_item = new BMenuItem("Accept", new BMessage(bmsgPrefGenCookieAccept)));

	if (NO_ACCEPT_COOKIE == pref->CookieAction())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Reject", new BMessage(bmsgPrefGenCookieReject)));
	if (NO_REJECT_COOKIE == pref->CookieAction())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Ask", new BMessage(bmsgPrefGenCookieAsk)));
	if (NO_ASK_COOKIE == pref->CookieAction())
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
	chkbox->SetValue((int)pref->LaunchDownloaded());
	AddChild(chkbox);

	r.OffsetBy(0,25);
	r.right -= 70; 
	char *days = new char[16]; // 16 digits should be enough
	sprintf(days, "%d", pref->DaysInGo());
	text = new BTextControl(r, "days_in_go", "Number of days to keep links in the Go menu:", days, new BMessage(bmsgPrefGenDaysInGo));
	text->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);
	text->SetDivider(230);
	AddChild(text);
}


// Display Properties
class DisplayView: public BView {
public:
	DisplayView(BRect r, Pref *);
};

DisplayView::DisplayView(BRect rect, Pref *pref) : BView(rect, "display_tab_view", B_FOLLOW_ALL, B_NAVIGABLE) {
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect r = Bounds();
	r.bottom = r.top + 15;
	float divider = 95; 

	BPopUpMenu *menu = new BPopUpMenu("encoding_menu");
	BMenuItem *menu_item;
	BMessage *msg;
	
	menu->AddItem(menu_item = new BMenuItem("Central European", msg = new BMessage(bmsgPrefDispEncoding)));
	msg->AddInt8("encoding", NO_CENTRAL_EUROPEAN);
	menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Cyrillic", msg = new BMessage(bmsgPrefDispEncoding)));
	msg->AddInt8("encoding", NO_CYRILLIC);
	menu->AddItem(menu_item = new BMenuItem("Greek", msg = new BMessage(bmsgPrefDispEncoding)));
	msg->AddInt8("encoding", NO_GREEK);
	menu->AddItem(menu_item = new BMenuItem("Japanese", msg = new BMessage(bmsgPrefDispEncoding)));
	msg->AddInt8("encoding", NO_JAPANESE);
	menu->AddItem(menu_item = new BMenuItem("Unicode", msg = new BMessage(bmsgPrefDispEncoding)));
	msg->AddInt8("encoding", NO_UNICODE);
	menu->AddItem(menu_item = new BMenuItem("Western", msg = new BMessage(bmsgPrefDispEncoding)));
	msg->AddInt8("encoding", NO_WESTERN);
	
	BMenuField *menu_field = new BMenuField(r, "for_encoding", "For the encoding:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(divider+2);
	AddChild(menu_field);

	r.OffsetBy(0,25);
	
	menu = new BPopUpMenu("proportional_menu");

	// NOTE: We select font settings for Central European, cause it is just a first item in a list
	int32 numFamilies = count_font_families();
	for ( int32 i = 0; i < numFamilies; i++ ) {
		font_family family;
		uint32 flags;
		if ( get_font_family(i, &family, &flags) == B_OK ) {
			menu->AddItem(menu_item = new BMenuItem(family, msg = new BMessage(bmsgPrefDispPropFont)));
			msg->AddString("family", family);

			if (0 ==i) menu_item->SetMarked(true); // select first one just in case there is no one set yet
			if (NULL != pref->FontFamily(NO_CENTRAL_EUROPEAN) &&
				0 == strcmp(family, pref->FontFamily(NO_CENTRAL_EUROPEAN)))
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
	if (7 == pref->FontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("9", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 9);
	if (9 == pref->FontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("10", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 10);
	if (10 == pref->FontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("12", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 12);
	if (12 == pref->FontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("14", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 14);
	if (14 == pref->FontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("18", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 18);
	if (18 == pref->FontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("24", msg = new BMessage(bmsgPrefDispPropSize)));
	msg->AddInt8("size", 24);
	if (24 == pref->FontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);

	menu_field = new BMenuField(r1, "prop_font_size", "Size:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(34);
	AddChild(menu_field);
	
	r1.OffsetBy(81,0);
	menu = new BPopUpMenu("prop_msize_menu");
	menu->AddItem(menu_item = new BMenuItem("7", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 7);
	if (7 == pref->FontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("9", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 9);
	if (9 == pref->FontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("10", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 10);
	if (10 == pref->FontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("12", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 12);
	if (12 == pref->FontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("14", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 14);
	if (14 == pref->FontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("18", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 18);
	if (18 == pref->FontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("24", msg = new BMessage(bmsgPrefDispPropMinSize)));
	msg->AddInt8("size", 24);
	if (24 == pref->FontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);

	menu_field = new BMenuField(r1, "prop_mfont_size", "Min. size:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(be_plain_font->StringWidth("Min. size:")+4);
	AddChild(menu_field);

	r.OffsetBy(0,25);
	
	menu = new BPopUpMenu("fixed_menu");

	for ( int32 i = 0; i < numFamilies; i++ ) {
		font_family family;
		uint32 flags;
		if ( get_font_family(i, &family, &flags) == B_OK ) {
			menu->AddItem(menu_item = new BMenuItem(family, msg = new BMessage(bmsgPrefDispFixedFont)));
			msg->AddString("family", family);
			if (0 ==i) menu_item->SetMarked(true); // select first one just in case there is no one set yet
			if (NULL != pref->FixedFontFamily(NO_CENTRAL_EUROPEAN) &&
				0 == strcmp(family, pref->FixedFontFamily(NO_CENTRAL_EUROPEAN)))
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
	if (7 == pref->FixedFontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("9", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 9);
	if (9 == pref->FixedFontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("10", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 10);
	if (10 == pref->FixedFontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("12", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 12);
	if (12 == pref->FixedFontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("14", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 14);
	if (14 == pref->FixedFontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("18", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 18);
	if (18 == pref->FixedFontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("24", msg = new BMessage(bmsgPrefDispFixedSize)));
	msg->AddInt8("size", 24);
	if (24 == pref->FixedFontSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);

	menu_field = new BMenuField(r1, "fixed_font_size", "Size:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(34);
	AddChild(menu_field);
	
	r1.OffsetBy(81,0);
	menu = new BPopUpMenu("fixed_msize_menu");
	menu->AddItem(menu_item = new BMenuItem("7", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 7);
	if (7 == pref->FixedFontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("9", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 9);
	if (9 == pref->FixedFontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("10", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 10);
	if (10 == pref->FixedFontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("12", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 12);
	if (12 == pref->FixedFontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("14", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 14);
	if (14 == pref->FixedFontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("18", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 18);
	if (18 == pref->FixedFontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("24", msg = new BMessage(bmsgPrefDispFixedMinSize)));
	msg->AddInt8("size", 24);
	if (24 == pref->FixedFontMinSize(NO_CENTRAL_EUROPEAN))
		menu_item->SetMarked(true);

	menu_field = new BMenuField(r1, "fixed_mfont_size", "Min. size:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(be_plain_font->StringWidth("Min. size:")+4);
	AddChild(menu_field);

	r.OffsetBy(0, 30);
	r.bottom = r.top+1;
	BBox *box = new BBox(r);
	AddChild(box);
	
	r.OffsetBy(0,5);
	r.right = Bounds().Width()/2 - 2;
	r1 = r; // save for future use
	BCheckBox *chkbox = new BCheckBox(r, 
		"show_images", 
		"Show images", 
		new BMessage(bmsgPrefDispShowImages));
	chkbox->SetValue((int)pref->ShowImages());
	AddChild(chkbox);

	r.OffsetBy(0,20);
	chkbox = new BCheckBox(r, 
		"show_bgimages", 
		"Show background images", 
		new BMessage(bmsgPrefDispShowBgImages));
	chkbox->SetValue((int)pref->ShowBgImages());
	AddChild(chkbox);

	r.OffsetBy(0,20);
	chkbox = new BCheckBox(r, 
		"show_animations", 
		"Show animations", 
		new BMessage(bmsgPrefDispShowAnimations));
	chkbox->SetValue((int)pref->ShowAnimations());
	AddChild(chkbox);

	r.OffsetBy(0,20);
	chkbox = new BCheckBox(r, 
		"underline_links", 
		"Underline links", 
		new BMessage(bmsgPrefDispUnderlineLinks));
	chkbox->SetValue((int)pref->UnderlineLinks());
	AddChild(chkbox);

	r.OffsetBy(0,20);
	chkbox = new BCheckBox(r, 
		"haiku_errors", 
		"Haiku error messages", 
		new BMessage(bmsgPrefDispHaikuErrors));
	chkbox->SetValue((int)pref->HaikuErrors());
	AddChild(chkbox);

	r = r1; // restore and move to right
	r.OffsetBy(Bounds().Width()/2-1, 0);
	r.right = r.left+1;
	r.bottom = Bounds().bottom-23;
	box = new BBox(r);
	AddChild(box);	 
	
	r.right = Bounds().right;
	r.OffsetBy(12, 0);
	r.bottom = r.top; 
	chkbox = new BCheckBox(r, 
		"use_fonts", 
		"Use fonts", 
		new BMessage(bmsgPrefDispUseFonts));
	chkbox->SetValue((int)pref->UseFonts());
	AddChild(chkbox);
		
	r.OffsetBy(0, 20);
	chkbox = new BCheckBox(r, 
		"use_bgcolors", 
		"Use background colors", 
		new BMessage(bmsgPrefDispUseBgColors));
	chkbox->SetValue((int)pref->UseBgColors());
	AddChild(chkbox);

	r.OffsetBy(0, 20);
	chkbox = new BCheckBox(r, 
		"use_fgcolors", 
		"Use foreground colors", 
		new BMessage(bmsgPrefDispUseFgColors));
	chkbox->SetValue((int)pref->UseFgColors());
	AddChild(chkbox);

	r.OffsetBy(0, 20);
	chkbox = new BCheckBox(r, 
		"play_sounds", 
		"Play sounds", 
		new BMessage(bmsgPrefDispPlaySounds));
	chkbox->SetValue((int)pref->PlaySounds());
	AddChild(chkbox);

	r.OffsetBy(0, 20);
	chkbox = new BCheckBox(r, 
		"flicker_free", 
		"Flicker-free drawing", 
		new BMessage(bmsgPrefDispFlickerFree));
	chkbox->SetValue((int)pref->FlickerFree());
	AddChild(chkbox);

}


// Connections Properties 
class ConnectionsView: public BView {
public:
	ConnectionsView(BRect r, Pref *);
private:
};

ConnectionsView::ConnectionsView(BRect rect, Pref *pref) : BView(rect, "connections_tab_view", B_FOLLOW_ALL, B_NAVIGABLE) {
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect r = Bounds();
	r.bottom = r.top+90;
	BBox *box = new BBox(r);
	AddChild(box);
	BCheckBox *chkbox = new BCheckBox(r, "enable_proxies", "Enable proxies", new BMessage(bmsgPrefConEnableProxies));
	chkbox->SetValue((int)pref->EnableProxies());
	box->SetLabel(chkbox);

	r.OffsetBy(11, 25);
	r.right = r.left + 256;
	BRect r1 = r; // save for next control below
	BTextControl *text = new BTextControl(r, CTL_HTTP_PROXY_NAME, "HTTP:", pref->HttpProxyName(), new BMessage(bmsgPrefConHttpProxyName)); 
	text->SetDivider(40);
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetEnabled(pref->EnableProxies());
	box->AddChild(text);
	
	r1.left = r1.right+12;
	r1.right = Bounds().right - 16;
	char *port = new char[16]; // 16 digits should be enough
	if (0 < pref->HttpProxyPort())
		sprintf(port, "%d", pref->HttpProxyPort());
	else 
		port = "";
	text = new BTextControl(r1, CTL_HTTP_PROXY_PORT, "Port:", port, new BMessage(bmsgPrefConHttpProxyPort)); 
	text->SetDivider(be_plain_font->StringWidth("Port:")+3);
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetEnabled(pref->EnableProxies());
	box->AddChild(text);
	
	r.OffsetBy(0, 30);
	text = new BTextControl(r, CTL_FTP_PROXY_NAME, "FTP:", pref->FtpProxyName(), new BMessage(bmsgPrefConFtpProxyName)); 
	text->SetDivider(40);
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetEnabled(pref->EnableProxies());
	box->AddChild(text);
	
	r1.OffsetBy(0,30);
	port = new char[16]; // 16 digits should be enough
	if (0 < pref->FtpProxyPort())
		sprintf(port, "%d", pref->FtpProxyPort());
	else 
		port = "";
	text = new BTextControl(r1, CTL_FTP_PROXY_PORT, "Port:", port, new BMessage(bmsgPrefConFtpProxyPort)); 
	text->SetDivider(be_plain_font->StringWidth("Port:")+3);
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetEnabled(pref->EnableProxies());
	box->AddChild(text);

	r.OffsetBy(0,45);
	r.left = 1;
	r.right=295;
	char *num = new char[16]; // 16 digits should be enough
	sprintf(num, "%d", pref->MaxConnections());

	text = new BTextControl(r, "max_connections", "Maximum number of simultaneous connections:", num, new BMessage(bmsgPrefConMaxConnections)); 
	text->SetDivider(be_plain_font->StringWidth("Maximum number of simultaneous connections:")+6);
	AddChild(text);
}

// Cache Properties
class CacheView: public BView {
public:
	CacheView(BRect r, Pref *);
};

CacheView::CacheView(BRect rect, Pref *pref) : BView(rect, "cache_tab_view", B_FOLLOW_ALL, B_NAVIGABLE) {
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect r = Bounds();
	r.bottom = r.top + 15;
	float divider = 90; //r.Width() / 2; // fixed divider
	BTextControl *text = new BTextControl(r, "cache_location", "Cache location:", pref->CacheLocation(), new BMessage(bmsgPrefCacheLocation)); 
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetDivider(divider);
	AddChild(text);
	
	r.OffsetBy(0,25);

	BPopUpMenu *menu = new BPopUpMenu("refresh_cache_menu");
	BMenuItem *menu_item;
	BMessage *msg;
	menu->AddItem(menu_item = new BMenuItem("Every time", msg = new BMessage(bmsgPrefCacheRefresh)));
	msg->AddInt32("value", (int) NO_REFRESH_EVERY_TIME); 
	if (NO_REFRESH_EVERY_TIME == pref->RefreshCache())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Once per session", msg = new BMessage(bmsgPrefCacheRefresh)));
	msg->AddInt32("value", (int) NO_REFRESH_ONCE_PER_SESSION); 
	if (NO_REFRESH_ONCE_PER_SESSION == pref->RefreshCache())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Once per day", msg = new BMessage(bmsgPrefCacheRefresh)));
	msg->AddInt32("value", (int) NO_REFRESH_ONCE_PER_DAY); 
	if (NO_REFRESH_ONCE_PER_DAY == pref->RefreshCache())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Never", msg = new BMessage(bmsgPrefCacheRefresh)));
	msg->AddInt32("value", (int) NO_REFRESH_NEVER); 
	if (NO_REFRESH_NEVER == pref->RefreshCache())
		menu_item->SetMarked(true);
	
	BMenuField *menu_field = new BMenuField(r, "refresh_cache", "Refresh cache:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(divider+2);
	AddChild(menu_field);

	r.OffsetBy(0,29);

	char size[32];
	sprintf(size, "%d", pref->CacheSize());
	text = new BTextControl(r, "cache_size", "Cache size (MB):", size, new BMessage(bmsgPrefCacheSize)); 
	text->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	text->SetDivider(divider);
	AddChild(text);

	r.OffsetBy(divider+2,25);
	r.right = r.left + 75;
	BButton *but = new BButton(r, "clear_now", "Clear now", new BMessage(bmsgPrefCacheClearNow));
	AddChild(but);
}

// Security Properties
class SecurityView: public BView {
public:
	SecurityView(BRect r, Pref *);
};

SecurityView::SecurityView(BRect rect, Pref *pref) : BView(rect, "security_tab_view", B_FOLLOW_ALL, B_NAVIGABLE) {
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect r = Bounds();

	BPopUpMenu *menu = new BPopUpMenu("unsecure_form_warning_menu");
	BMenuItem *menu_item;
	BMessage *msg;
	menu->AddItem(menu_item = new BMenuItem("Never", msg = new BMessage(bmsgPrefSecWarnOnSend)));
	msg->AddInt32("value", (int) NO_WARN_NEVER); 
	if (NO_WARN_NEVER == pref->UnsecureFormWarning())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("When sending more than one line", msg = new BMessage(bmsgPrefSecWarnOnSend)));
	msg->AddInt32("value", (int) NO_WARN_MORE_THAN_ONE_LINE); 
	if (NO_WARN_MORE_THAN_ONE_LINE == pref->UnsecureFormWarning())
		menu_item->SetMarked(true);
	menu->AddItem(menu_item = new BMenuItem("Always", msg = new BMessage(bmsgPrefSecWarnOnSend)));
	msg->AddInt32("value", (int) NO_WARN_ALWAYS); 
	if (NO_WARN_ALWAYS== pref->UnsecureFormWarning())
		menu_item->SetMarked(true);
	
	BMenuField *menu_field = new BMenuField(r, "unsecure_form_warning", "Warn when sending an unsecure form:", menu);
	menu_field->SetAlignment(B_ALIGN_RIGHT);
	menu_field->SetDivider(be_plain_font->StringWidth("Warn when sending an unsecure form:")+3);
	AddChild(menu_field);

	r.OffsetBy(0,22);
	r.bottom = r.top+1;
	BCheckBox *chkbox = new BCheckBox(r, 
		"warn_enter", 
		"Warn when entering secure site", 
		new BMessage(bmsgPrefSecEnterSite));
	chkbox->SetValue((int)pref->WarnEnterSecureSite());
	AddChild(chkbox);

	r.OffsetBy(0,20);
	chkbox = new BCheckBox(r, 
		"warn_leave", 
		"Warn when leaving secure site", 
		new BMessage(bmsgPrefSecLeaveSite));
	chkbox->SetValue((int)pref->WarnLeaveSecureSite());
	AddChild(chkbox);

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

	// init settings
	m_pref = new Pref(Pref::Default);
	BRect r = Bounds();
	r.InsetBy(-1,-1);
	BBox *b = new BBox(r);
	r.left += 2;
	r.top += 13;
	r.bottom -= 45;
	TabView *tab_view = new TabView(r, "tab_view");
	tab_view->SetTabWidth(B_WIDTH_AS_USUAL);
	r = tab_view->ChildAt(0)->Bounds(); // get bounds of internal view
	NarrowTab *tab = new NarrowTab();
	r.InsetBy(13,9);
	tab_view->AddTab(new GeneralView(r, m_pref), tab);
	tab->SetLabel("General");
	tab = new NarrowTab();
	tab_view->AddTab(new DisplayView(r, m_pref), tab);
	tab->SetLabel("Display");	
	tab = new NarrowTab();
	tab_view->AddTab(new ConnectionsView(r, m_pref), tab);
	tab->SetLabel("Connections");	
	tab = new NarrowTab();
	tab_view->AddTab(new CacheView(r, m_pref), tab);
	tab->SetLabel("Cache");	
	tab = new NarrowTab();
	r.InsetBy(-3,0);
	tab_view->AddTab(new SecurityView(r, m_pref), tab);
	tab->SetLabel("Security");	
	b->AddChild(tab_view);

	// draw buttons now
	r = b->Bounds();
	r.top = r.bottom - 36; 
	r.bottom = r.top + 20;
	r.left = 231;
	r.right = r.left + 75;
	FocusButton *but = new FocusButton(r, "cancel_button", "Cancel", new BMessage(B_CANCEL));
	b->AddChild(but);
	r.OffsetBy(75+10, 0);
	but = new FocusButton(r, "ok_button", "OK", new BMessage(B_OK));
	but->MakeDefault(true);
	b->AddChild(but);
	AddChild(b);
	
}


// Message handler
void PrefWindow::MessageReceived(BMessage *msg) {

	BTextControl *text = NULL;
	BCheckBox *chkbox = NULL;

	switch (msg->what) {
		case bmsgPrefGenHomePage:
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetHomePage(text->Text());
			break;
		case bmsgPrefGenSearchPage:
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetSearchPage(text->Text());
			break;
		case bmsgPrefGenDownloadDirectory:
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetDownloadDirectory(text->Text());
			break;
		case bmsgPrefGenCloneWindow:
			m_pref->SetNewWindowAction(NO_CLONE_CURRENT);
			break;
		case bmsgPrefGenOpenHome:
			m_pref->SetNewWindowAction(NO_OPEN_HOME);
			break;
		case bmsgPrefGenOpenBlank:
			m_pref->SetNewWindowAction(NO_OPEN_BLANK);
			break;
		case bmsgPrefGenCookieAccept:
			m_pref->SetCookieAction(NO_ACCEPT_COOKIE);
			break;
		case bmsgPrefGenCookieReject	:
			m_pref->SetCookieAction(NO_REJECT_COOKIE);
			break;
		case bmsgPrefGenCookieAsk:
			m_pref->SetCookieAction(NO_ASK_COOKIE);
			break;
		case bmsgPrefGenAutoLaunch: {
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetLaunchDownloaded((bool)chkbox->Value());
			break;
		}
		case bmsgPrefGenDaysInGo:
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetDaysInGo(atoi(text->Text()));
			break;
		case bmsgPrefDispEncoding: {
			display_encoding de;
			if (B_OK != msg->FindInt8("encoding", (int8 *) &de))
				break;
			// FIXME:
			// 1. get prop font family name
			// 2. scan the menu and select one
			// 3. get prop font sizes
			// 4. scan the menus and select
			// do the same for fixed font
			break;
		}
		// FIXME:
		case bmsgPrefDispPropFont:
			int8 size;
			if (B_OK != msg->FindInt8("size", (int8) &size))
				break;
			
			break;
		case bmsgPrefDispPropSize:
			break;
		case bmsgPrefDispPropMinSize:
			break;
		case bmsgPrefDispFixedFont:
			break;
		case bmsgPrefDispFixedSize:
			break;
		case bmsgPrefDispFixedMinSize:
			break;

		case bmsgPrefDispShowImages:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetShowImages((bool)chkbox->Value());
			break;
		case bmsgPrefDispShowBgImages:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetShowBgImages((bool)chkbox->Value());
			break;
		case bmsgPrefDispShowAnimations:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetShowAnimations((bool)chkbox->Value());
			break;
		case bmsgPrefDispUnderlineLinks:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetUnderlineLinks((bool)chkbox->Value());
			break;
		case bmsgPrefDispHaikuErrors:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetHaikuErrors((bool)chkbox->Value());
			break;
		case bmsgPrefDispUseFonts:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetUseFonts((bool)chkbox->Value());
			break;
		case bmsgPrefDispUseBgColors:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetUseBgColors((bool)chkbox->Value());
			break;
		case bmsgPrefDispUseFgColors:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetUseFgColors((bool)chkbox->Value());
			break;
		case bmsgPrefDispPlaySounds:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetPlaySounds((bool)chkbox->Value());
			break;
		case bmsgPrefDispFlickerFree:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetFlickerFree((bool)chkbox->Value());
			break;
		case bmsgPrefConEnableProxies:
			text = (BTextControl *) FindView(CTL_HTTP_PROXY_NAME);
			if (text) text->SetEnabled(!text->IsEnabled());
			text = (BTextControl *) FindView(CTL_HTTP_PROXY_PORT);
			if (text) text->SetEnabled(!text->IsEnabled());
			text = (BTextControl *) FindView(CTL_FTP_PROXY_NAME);
			if (text) text->SetEnabled(!text->IsEnabled());
			text = (BTextControl *) FindView(CTL_FTP_PROXY_PORT);
			if (text) text->SetEnabled(!text->IsEnabled());

			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetEnableProxies((bool)chkbox->Value());
			break;
		case bmsgPrefConHttpProxyName:
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetHttpProxyName(text->Text());
			break;
		case bmsgPrefConHttpProxyPort:
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetHttpProxyPort(atoi(text->Text()));
			break;
		case bmsgPrefConFtpProxyName:
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetFtpProxyName(text->Text());
			break;
		case bmsgPrefConFtpProxyPort:
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetFtpProxyPort(atoi(text->Text()));
			break;
		case bmsgPrefConMaxConnections:
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetMaxConnections(atoi(text->Text()));
			break;
		case bmsgPrefCacheLocation: 
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetCacheLocation(text->Text());
			break;
		case bmsgPrefCacheRefresh: {
			refresh_cache rc;
			if (B_OK == msg->FindInt32("value", (int32*) &rc))
				m_pref->SetRefreshCache(rc);
			break;
		}
		case bmsgPrefCacheSize: 
			if (B_OK == msg->FindPointer("source", (void **) &text))
				m_pref->SetCacheSize(atoi(text->Text()));
			break;
		case bmsgPrefCacheClearNow:
			// FIXME: add cache clearing here
			// should be very simple: remove all entries in the directory CacheLocation
			//
			break;
		case bmsgPrefSecWarnOnSend: {
			unsecure_form_warning w;
			if (B_OK == msg->FindInt32("value", (int32*) &w))
				m_pref->SetUnsecureFormWarning(w);
			break;
		}
		case bmsgPrefSecEnterSite:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetWarnEnterSecureSite((bool)chkbox->Value());
			break;
		case bmsgPrefSecLeaveSite:
			if (B_OK == msg->FindPointer("source", (void **) &chkbox))
				m_pref->SetWarnLeaveSecureSite((bool)chkbox->Value());
			break;
		case B_OK:
			Pref::Default = (*m_pref);
			Pref::Default.Save();
		case B_CANCEL:
			Quit();
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}
