#include <Application.h>
#include <Roster.h>
#include <Screen.h>
#include <ScrollView.h>
#include <be/interface/MenuBar.h>
#include <MenuItem.h>
#include <UTF8.h>
#ifdef __BEOS__
#include <be/kernel/fs_attr.h>
#endif
#include <be/storage/Directory.h>
#include <be/storage/Entry.h>
#include <be/storage/Path.h>
#include <be/support/SupportDefs.h>
#ifdef __BEOS__
#include <be/storage/FilePanel.h>
#endif
#include <TextControl.h>
#include <Shelf.h>
#include "NOWindow.h"      
#include "EventCodes.h"
#include "ResourceGetter.h"
#include "Html.h"
#include "HTMLView.h"
#include "ToolBarView.h"
#include "LinkBarView.h"
#include "StatusBarView.h"
#include "traces.h"
#include "Pref.h"
#include "PrefWindow.h"
#include "UIMessages.h"
#include "Url.h"
#include "History.h"
#include "UrlText.h"

#define bmsgForceTableBorder 'NOTB'
#define bmsgExportToFwt 'EFWT'
#define bmsgNoBackground 'NBGC'


// FIXME: should be changed to our own folders later
#define NO_BOOKMARKS_PATH	"/boot/home/config/settings/NetPositive/Bookmarks"
#define NO_HISTORY_PATH "/boot/home/config/settings/NetPositive/History"


static void buildBookmarksMenu(BMenu *menu, const char *dirname) {
	// Read bookmarks
	BDirectory dir(dirname);
	int32 count = dir.CountEntries();
	BEntry entry;
	bool anyDirectories = false;	
	for(int32 i=0; i<count; i++) {
		if(B_ENTRY_NOT_FOUND == dir.GetNextEntry(&entry, true))
			break;

		BNode node(&entry);
		char url[256];		
		char name[256];
		if(0 < node.ReadAttr("META:url", B_STRING_TYPE, 0, url, sizeof url-1)
			&& 0 < node.ReadAttr("META:title", B_STRING_TYPE, 0, name, sizeof name-1)) {
			BMessage *msg = new BMessage(bmsgBookmarkSelected);
			msg->AddString("url", url);
			menu->AddItem(new BMenuItem(name, msg));
		}
		if (node.IsDirectory())
			anyDirectories = true;
	}
	if (anyDirectories) {
		dir.Rewind();
		menu->AddSeparatorItem();
		for(int32 i=0; i<count; i++) {
			if(B_ENTRY_NOT_FOUND == dir.GetNextEntry(&entry, true))
				break;
			BNode node(&entry);
			if (node.IsDirectory()) {
				char name[B_FILE_NAME_LENGTH];
				BPath path; 
				entry.GetPath(&path);
				entry.GetName(name);
				BMenu *submenu = new BMenu(name);
				buildBookmarksMenu(submenu, path.Path());
				menu->AddItem(submenu);
			}
		}
	}
}

NOWindow::NOWindow(BRect windowfr) : BWindow(windowfr, "NetOptimist", B_DOCUMENT_WINDOW, /*B_OUTLINE_RESIZE|*/B_ASYNCHRONOUS_CONTROLS),
		ToolBarViewHeigth(30), m_fullScreen(false), fileOpenPanel_(NULL)
{
	// Créer le menu.
	BRect r = Bounds();
	r.bottom = r.top+15;
	
	m_menu = new BMenuBar(r,"MenuBar");
	AddChild(m_menu);
	
	BMenuItem *menuitem;
	BMenu *submenu;
	
	// Le menu File
	submenu = new BMenu("File");
	submenu->AddItem(menuitem=new BMenuItem("New",new BMessage(bmsgFileNew),'N',B_COMMAND_KEY));
	submenu->AddItem(menuitem=new BMenuItem("Open Location...",new BMessage(bmsgFileOpenLocation),'L',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem("Open File...",new BMessage(bmsgFileOpenFile),'O',B_COMMAND_KEY));
	submenu->AddSeparatorItem();
	submenu->AddItem(menuitem=new BMenuItem("Save As",new BMessage(bmsgFileSaveAs),'S',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem("Close",new BMessage(bmsgFileClose),'W',B_COMMAND_KEY));
//	menuitem->SetEnabled(false);
	submenu->AddSeparatorItem();
	submenu->AddItem(menuitem=new BMenuItem("Page Setup",new BMessage(bmsgFilePageSetup),'P',B_SHIFT_KEY|B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem("Print",new BMessage(bmsgFilePrint),'P',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddSeparatorItem();
	submenu->AddItem(menuitem=new BMenuItem("About NetOptimist",new BMessage(B_ABOUT_REQUESTED)));
	menuitem->SetEnabled(false);
	submenu->AddSeparatorItem();
	submenu->AddItem(new BMenuItem("Quit",new BMessage(B_QUIT_REQUESTED),'Q',B_COMMAND_KEY));
	m_menu->AddItem(submenu);
	// Le menu Edit
	submenu = new BMenu("Edit");
	//submenu->SetEnabled(false);
/*	submenu->AddItem(menuitem=new BMenuItem( "Undo",new BMessage(bmsgEditUndo),'Z',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddSeparatorItem();
	submenu->AddItem(menuitem=new BMenuItem( "Cut",new BMessage(bmsgEditCut),'X',B_COMMAND_KEY));
	//menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem( "Copy",new BMessage(bmsgEditCopy),'C',B_COMMAND_KEY));
	//menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem( "Paste",new BMessage(bmsgEditPaste),'V',B_COMMAND_KEY));
	//menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem( "Clear",new BMessage(bmsgEditClear)));
	menuitem->SetEnabled(false);
	submenu->AddSeparatorItem();
	submenu->AddItem(menuitem=new BMenuItem( "Select All",new BMessage(bmsgEditSelectAll),'A',B_COMMAND_KEY));
	//menuitem->SetEnabled(false);
	submenu->AddSeparatorItem();
*/
	submenu->AddItem(menuitem=new BMenuItem("Find",new BMessage(bmsgEditFind),'F',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem("Find Again",new BMessage(bmsgEditFindAgain),'G',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddSeparatorItem();
	submenu->AddItem(menuitem=new BMenuItem("Preferences...",new BMessage(bmsgEditPreferences)));
	m_menu->AddItem(submenu);
	// Le menu Go
	submenu = new BMenu("Go");
	submenu->AddItem(menuitem = new BMenuItem("Back",new BMessage(bmsgButtonBACK),B_LEFT_ARROW,B_COMMAND_KEY));
	submenu->AddItem(menuitem = new BMenuItem("Forward",new BMessage(bmsgButtonNEXT),B_RIGHT_ARROW,B_COMMAND_KEY));
	submenu->AddItem(menuitem=new BMenuItem("Home",new BMessage(bmsgButtonHOME),'H',B_COMMAND_KEY));
	submenu->AddItem(menuitem=new BMenuItem("Search",new BMessage(bmsgGoSearch),'S',B_SHIFT_KEY|B_COMMAND_KEY));
	submenu->AddSeparatorItem();
	buildBookmarksMenu(submenu, NO_HISTORY_PATH);
	m_menu->AddItem(submenu);

	// Le menu Bookmarks
	BMenu *m_bookmarks;
	m_bookmarks = new BMenu("Bookmarks");
	m_bookmarks->AddItem(menuitem=new BMenuItem("Add to Bookmarks",new BMessage(bmsgBookmarksAdd),'B',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	m_bookmarks->AddItem(menuitem=new BMenuItem("Show Bookmarks",new BMessage(bmsgBookmarksShow)));
	m_bookmarks->AddSeparatorItem();
	buildBookmarksMenu(m_bookmarks, NO_BOOKMARKS_PATH);
	// Add Bookmarks submenu
	m_menu->AddItem(m_bookmarks);
	
	// Le menu View
	submenu = new BMenu("View");
	submenu->AddItem(menuitem=new BMenuItem( "Full Screen",new BMessage(bmsgViewFullscreen)));
	submenu->AddSeparatorItem();
	submenu->AddItem(menuitem=new BMenuItem( "Download Window",new BMessage(bmsgViewDownload),'D',B_SHIFT_KEY|B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddSeparatorItem();
	submenu->AddItem(menuitem=new BMenuItem( "Show HTML",new BMessage(bmsgViewHtml),'H',B_SHIFT_KEY|B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem( "Reload Page",new BMessage(bmsgViewReload),'R',B_COMMAND_KEY));

	// Add "Document Encodings" submenu
	submenu->AddSeparatorItem();
	BMenu *submenu1 = new BMenu("Document Encoding");
	submenu1->AddItem(menuitem=new BMenuItem("Auto Detect", new BMessage(bmsgViewEncAuto)));
	menuitem->SetEnabled(false);
	submenu1->AddSeparatorItem();
	BMessage *msg;
	int enc = Pref::Default.Encoding();
	submenu1->AddItem(menuitem=new BMenuItem("Central European (ISO 8859-2)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_ISO2_CONVERSION);
	if (B_ISO2_CONVERSION == enc) menuitem->SetMarked(true);
	submenu1->AddItem(menuitem=new BMenuItem("Cyrillic (ISO 8859-5)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_ISO5_CONVERSION);
	if (B_ISO5_CONVERSION == enc) menuitem->SetMarked(true);
	submenu1->AddItem(menuitem=new BMenuItem("Cyrillic (KOI8-R)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_KOI8R_CONVERSION);
	if (B_KOI8R_CONVERSION == enc) menuitem->SetMarked(true);
	submenu1->AddItem(menuitem=new BMenuItem("Cyrillic (MS-DOS 866)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_MS_DOS_866_CONVERSION);
	if (B_MS_DOS_866_CONVERSION == enc) menuitem->SetMarked(true);
	submenu1->AddItem(menuitem=new BMenuItem("Cyrillic (Windows 1251)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_MS_WINDOWS_1251_CONVERSION);
	if (B_MS_WINDOWS_1251_CONVERSION == enc) menuitem->SetMarked(true);
	submenu1->AddItem(menuitem=new BMenuItem("Greek (ISO 8859-7)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_ISO7_CONVERSION);
	if (B_ISO7_CONVERSION == enc) menuitem->SetMarked(true);
	submenu1->AddItem(menuitem=new BMenuItem("Japanese (Auto Detect)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_JIS_CONVERSION); // FIXME: NEXUS: I'm certainly NOT SURE about this one
	if (B_JIS_CONVERSION == enc) menuitem->SetMarked(true);
	submenu1->AddItem(menuitem=new BMenuItem("Japanese (Shift-JIS)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_SJIS_CONVERSION);
	if (B_SJIS_CONVERSION == enc) menuitem->SetMarked(true);
	submenu1->AddItem(menuitem=new BMenuItem("Japanese (EUC)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_EUC_CONVERSION);
	if (B_EUC_CONVERSION == enc) menuitem->SetMarked(true);
	submenu1->AddItem(menuitem=new BMenuItem("Unicode", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_UNICODE_CONVERSION);
	if (B_UNICODE_CONVERSION == enc) menuitem->SetMarked(true);
/*	
	// NEXUS: I don't really know what to do with it. Is there a situation where such conversion available?
	submenu1->AddItem(menuitem=new BMenuItem("Unicode (UTF-8)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_UTF8_CONVERSION); 
	if (B_ISO1_CONVERSION == enc) menuitem->SetMarked(true);
*/
	submenu1->AddItem(menuitem=new BMenuItem("Western (ISO 8859-1)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_ISO1_CONVERSION);
	if (B_ISO1_CONVERSION == enc) menuitem->SetMarked(true);
	submenu1->AddItem(menuitem=new BMenuItem("Western (Mac Roman)", msg = new BMessage(bmsgViewSetEncoding)));
	msg->AddInt32("encoding", B_MAC_ROMAN_CONVERSION);
	if (B_MAC_ROMAN_CONVERSION == enc) menuitem->SetMarked(true);
	submenu->AddItem(submenu1);
	
	m_menu->AddItem(submenu);
	
	// Le menu Debug
	submenu = new BMenu("Debug");
	submenu->AddItem(m_debugForceTableBorder=new BMenuItem( "Force table borders",new BMessage(bmsgForceTableBorder)));
	m_debugForceTableBorder->SetMarked(ISTRACE(DEBUG_FORCETABLEBORDER));
	submenu->AddItem(m_debugExportFWTFile=new BMenuItem( "Export HTML to FWT file",new BMessage(bmsgExportToFwt)));
	m_debugExportFWTFile->SetMarked(ISTRACE(WRITE_FWT_FILE));
	submenu->AddItem(m_debugNoFillRect=new BMenuItem( "No table background color",new BMessage(bmsgNoBackground)));
	m_debugNoFillRect->SetMarked(! ISTRACE(FILLRECT));
	m_menu->AddItem(submenu);
	
	r = Bounds();
	r.top+=m_menu->Frame().bottom+1;
	r.bottom=r.top+32;
	toolBarView = new ToolBarView(r, this);
#ifdef __BEOS__
	BShelf *fShelf = new BShelf(toolBarView);
	fShelf->SetDisplaysZombies(true);
#endif
	AddChild(toolBarView);
	
	urlControl = new UrlText(BRect(30,5,250,20), BRect(0,0,300,30));
	toolBarView->AddChild(urlControl);
	
	int const LinkBarHeight = 15;
	
	r = Bounds();
	r.top+=toolBarView->Frame().bottom+3;
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.bottom -= B_H_SCROLL_BAR_HEIGHT * 2 + LinkBarHeight;

	drawArea = new HTMLView (this, r);
	scrollView = new BScrollView("HtmlScroll", drawArea, B_FOLLOW_ALL_SIDES, B_NAVIGABLE, true, true);
	AddChild(scrollView);
	drawArea->MakeFocus();
	
	rgb_color bgcol = ui_color(B_PANEL_BACKGROUND_COLOR);
	
	// The link bar
	r = Bounds();
	r.bottom -= B_H_SCROLL_BAR_HEIGHT;
	r.top = r.bottom - LinkBarHeight;
	m_linkBarView = new LinkBarView(r, "LinkBar", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, 0);
	m_linkBarView->SetViewColor(bgcol);
	m_linkBarView->SetLowColor(bgcol);
	AddChild(m_linkBarView);

	// The status bar
	r = Bounds();
	r.top = r.bottom - B_H_SCROLL_BAR_HEIGHT;
	m_statusView = new StatusBarView(r, "StatusView", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_FRAME_EVENTS);
	m_statusView->SetViewColor(bgcol);
	m_statusView->SetLowColor(bgcol);
	AddChild(m_statusView);
	
	drawArea->SetStatusBarView(m_statusView);
	UpdateNavControls();
	
#ifdef __BEOS__
	fileOpenPanel_ = new BFilePanel;
#endif
}

// do some clean-ups here...
void NOWindow::Quit() {
	if (fileOpenPanel_) 	DELETE(fileOpenPanel_);
	BWindow::Quit();
}

void NOWindow::IsDownloading(bool set) {
	toolBarView->SetEnabled(bmsgButtonSTOP, set);
}

void NOWindow::SetUrl(const char *url) {
	if (Lock()) {
		drawArea->SetUrl(url);
		Unlock();
	}
}
	
void NOWindow::UrlChanged(const char *str) {
	urlControl->SetText(str);
	if (m_linkBarView) { 
		m_linkBarView->AddLinkFromDocument(&drawArea->m_document);
		m_linkBarView->Invalidate();
	}
	fprintf(stderr, "Url changed to %s\n", str);
	toolBarView->SetDocumentIcon(drawArea->m_document.Icon());
}


// Updates back and forward buttons and menu items state
void NOWindow::UpdateNavControls() {
	bool back = History::history.HasBack();
	bool forward = History::history.HasForward();
	fprintf(stderr, "*** Updating nav.controls: Back (%u), Fwd (%u)\n", back, forward);
	toolBarView->SetEnabled(bmsgButtonBACK, back);
	toolBarView->SetEnabled(bmsgButtonNEXT, forward);
#ifdef __BEOS__
	m_menu->FindItem(bmsgButtonBACK)->SetEnabled(back);
	m_menu->FindItem(bmsgButtonNEXT)->SetEnabled(forward);
#endif
}


void NOWindow::MessageReceived(BMessage *message) {
	switch (message->what) {
		case URL_ENTERED:
			fprintf(stderr, "User asks %s\n", urlControl->Text());
			if (Lock()) {
				drawArea->SetUrl(urlControl->Text());
				Unlock();
				UpdateNavControls();
			}
			break;
		case URL_MODIFIED:
			const char *str;
			message->FindString("URL", &str);
			UrlChanged(str);
			UpdateNavControls();
			break;
		case bmsgFileClose:
			PostMessage(new BMessage(B_CLOSE_REQUESTED));
			break;
		case B_CLOSE_REQUESTED:
			fprintf(stderr, "B_CLOSE_REQUESTED NOWindow\n");
			break;
		case B_ABOUT_REQUESTED:
			fprintf(stderr, "about NOWindow\n");
			break;
		case B_SIMPLE_DATA:
		case B_MIME_DATA:
			{
#ifdef __BEOS__
#if B_BEOS_VERSION >= 0x0510
				const char *name; // BMessage::GetInfo changed in Dano : it now requires a const
#else
				char *name;
#endif
				uint32 type;
				int32 count;
				for ( int32 i = 0;
					  message->GetInfo(B_ANY_TYPE, i, &name, &type, &count) == B_OK;
					  i++ ) {
					  fprintf(stderr, "Data message : %s type %x\n", name, (int)type);
				}
				if (message->FindString("be:url", &str)==B_OK) {
					fprintf(stderr, "Drag&drop url %s\n", str);
					urlControl->SetText(str);
					SetUrl(str);
				} else 	{
					entry_ref ref;
					if (message->FindRef("refs", &ref)==B_OK) {
						fprintf(stderr, "Drag&drop File %s\n", ref.name);
						BNode node(&ref);
						char attrName[B_ATTR_NAME_LENGTH];
						while (node.GetNextAttrName(attrName)==B_OK) {
							fprintf(stderr, "> Attr name: %s\n", attrName);
						}
						char urlName[256];
						if (node.ReadAttr("META:url", B_STRING_TYPE, 0, urlName, sizeof urlName)>0) {
							fprintf(stderr, "> Attr name: %s\n", urlName);
						}
						urlControl->SetText(urlName);
						SetUrl(urlName);
					}
				}
				UpdateNavControls();
#endif
			}
			break;
		case URL_LOADED:
			trace(DEBUG_MESSAGING)
				fprintf(stdout, "######## Message REFORMAT received in NO Window\n");
			{
				drawArea->CheckUrl();
				bool reformat = false;
				message->FindBool("reformat", &reformat);
				if (reformat)
					drawArea->Redraw();
				else
					drawArea->Refresh();
			}
			break;
		case bmsgButtonBACK:
			drawArea->Back();
			UpdateNavControls();
			break;
		case bmsgButtonNEXT:
			drawArea->Forward();
			UpdateNavControls();
			break;
		case bmsgButtonHOME:
			SetUrl(Pref::Default.HomePage());
			UpdateNavControls();
			break;
		case bmsgGoSearch:
			SetUrl(Pref::Default.SearchPage());
			UpdateNavControls();
			break;
		case bmsgBookmarkSelected: {
			char *url = new char[256];
			if (B_OK == message->FindString("url", (const char **)&url)) {
				SetUrl(url);
				UpdateNavControls();
			}
			//delete url;
			break;
		}
		case bmsgViewReload:
		case bmsgButtonRELOAD:
			Cache::cache.ClearAll();
			SetUrl(this->urlControl->Text());
			break;

#ifdef __BEOS__
		case bmsgEditPreferences: {
			PrefWindow *wnd = new PrefWindow();
			wnd->Show();
			break;
		}
		case bmsgBookmarksShow: {
			char *str = strdup(NO_BOOKMARKS_PATH);
			be_roster->Launch("application/x-vnd.Be-TRAK", 
				1,  &str);
			FREE(str);
			break;
		}
		
		case bmsgViewSetEncoding: {
			int32 encoding = 0;
			if (B_OK == message->FindInt32("encoding", &encoding))
				encoding = SetEncoding(encoding);
			BMenuItem *item;
			if ((B_OK == message->FindPointer("source", (void **)&item)) && item) {
				BMenuItem *prevItem = item->Menu()->FindMarked();
				if (prevItem) prevItem->SetMarked(false);
				item->SetMarked(true);
			}
			break;
		}
#endif
		
		case bmsgFontsChanged: {
			drawArea->SetSourceEncoding(drawArea->SourceEncoding()); // need to reload the fonts!
			SetUrl(this->urlControl->Text());
			break;
		}
			
		case bmsgViewFullscreen: {
			FullScreen();
			break;
		}	

		case bmsgFileNew: {
			BRect newFrame = Frame();
			newFrame.OffsetBySelf(30,30);
			NOWindow *newwin = new NOWindow(newFrame);
			newwin->Show();
			break;
		}

#ifdef __BEOS__
		case bmsgFileOpenFile:
			fileOpenPanel_->Show();
			break;
#endif

		case bmsgForceTableBorder:
			if (ISTRACE(DEBUG_FORCETABLEBORDER))
				enableTrace &= ~DEBUG_FORCETABLEBORDER;
			else
				enableTrace |= DEBUG_FORCETABLEBORDER;
			m_debugForceTableBorder->SetMarked(ISTRACE(DEBUG_FORCETABLEBORDER));
			drawArea->Refresh();
			break;
		case bmsgNoBackground:
			if (ISTRACE(FILLRECT))
				enableTrace &= ~FILLRECT;
			else
				enableTrace |= FILLRECT;
			m_debugNoFillRect->SetMarked(! ISTRACE(FILLRECT));
			drawArea->Refresh();
			break;
		case bmsgExportToFwt:
			if (ISTRACE(WRITE_FWT_FILE))
				enableTrace &= ~WRITE_FWT_FILE;
			else
				enableTrace |= WRITE_FWT_FILE;
			m_debugExportFWTFile->SetMarked(ISTRACE(WRITE_FWT_FILE));
			break;
		default:
			trace(DEBUG_MESSAGING)
				fprintf(stderr, "Message received %4.4s in NO Window\n", (char *)&message->what);
			BWindow::MessageReceived(message);
	}
}

void NOWindow::FullScreen() {
	if (m_fullScreen) {
		MoveTo(m_orgFrame.left, m_orgFrame.top);
		ResizeTo(m_orgFrame.Width(), m_orgFrame.Height());
		SetFlags(Flags() ^ (B_NOT_RESIZABLE | B_NOT_MOVABLE));
	} else {
		m_orgFrame = Frame();
		BScreen *scr = new BScreen(this);
		BRect r = scr->Frame();
		MoveTo(0, 0);
		ResizeTo(r.Width(), r.Height());
		SetFlags(Flags() | B_NOT_RESIZABLE | B_NOT_MOVABLE);
	}
	m_fullScreen = !m_fullScreen;
}

bool NOWindow::QuitRequested() {
	if (be_app->CountWindows()<=3) { // now, one of them is BFilePanel which I create, what is another one?
		// This is the last window
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
	return(true);
}

uint32 NOWindow::SetEncoding(uint32 enc) {
	uint32 prev = 0;
	if (drawArea) {
		prev = drawArea->SourceEncoding();
		drawArea->SetSourceEncoding(enc);
		SetUrl(this->urlControl->Text()); // reload
		Pref::Default.SetEncoding(enc);
	}
	return prev;
}

HTMLFrame *NOWindow::MainFrame() {
	return drawArea;
}
