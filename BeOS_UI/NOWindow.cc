#include <Application.h>
#include <Screen.h>
#include <be/interface/MenuBar.h>
#include <MenuItem.h>
#include <be/kernel/fs_attr.h>
#include <be/storage/Entry.h>
#include <be/storage/Directory.h>
#include <be/storage/Entry.h>
#include <TextControl.h>
#include <Shelf.h>
#include "NOWindow.h"      
#include "EventCodes.h"
#include "ResourceGetter.h"
#include "LinkBarView.h"
#include "StatusBarView.h"
#include "traces.h"
#include "Pref.h"
#include "PrefWindow.h"
#include "UIMessages.h"
#include "Url.h"
#include "History.h"

#define bmsgForceTableBorder 'NOTB'
#define bmsgExportToFwt 'EFWT'
#define bmsgNoBackground 'NBGC'

NOWindow::NOWindow(BRect windowfr) : BWindow(windowfr, "NetOptimist", B_DOCUMENT_WINDOW, /*B_OUTLINE_RESIZE|*/B_ASYNCHRONOUS_CONTROLS),
		ToolBarViewHeigth(30), m_fullScreen(false)
{
	// Créer le menu.
	BRect r = Bounds();
	r.bottom = r.top+15;
	
	m_menu = new BMenuBar(r,"MenuBar");
	AddChild(m_menu);
	
	BMenuItem *menuitem;
	//BMenu *subsubmenu; 
	BMenu *submenu;
	
	// Le menu File
	submenu = new BMenu("File");
	submenu->AddItem(menuitem=new BMenuItem("New",new BMessage(bmsgFileNew),'N',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem("Open Location...",new BMessage(bmsgFileOpenLocation),'L',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem("Open File...",new BMessage(bmsgFileOpenFile),'O',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
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
*/
	submenu->AddSeparatorItem();
	submenu->AddItem(menuitem=new BMenuItem("Find",new BMessage(bmsgEditFind),'F',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddItem(menuitem=new BMenuItem("Find Again",new BMessage(bmsgEditFindAgain),'G',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddSeparatorItem();
	submenu->AddItem(menuitem=new BMenuItem("Preferences...",new BMessage(bmsgEditPreferences)));
//	menuitem->SetEnabled(false);
	m_menu->AddItem(submenu);
	// Le menu Go
	submenu = new BMenu("Go");
	submenu->AddItem(menuitem = new BMenuItem("Back",new BMessage(bmsgButtonBACK),B_LEFT_ARROW,B_COMMAND_KEY));
	submenu->AddItem(menuitem = new BMenuItem("Forward",new BMessage(bmsgButtonNEXT),B_RIGHT_ARROW,B_COMMAND_KEY));
	submenu->AddItem(menuitem=new BMenuItem("Home",new BMessage(bmsgButtonHOME),'H',B_COMMAND_KEY));
	submenu->AddItem(menuitem=new BMenuItem("Search",new BMessage(bmsgGoSearch),'S',B_SHIFT_KEY|B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	submenu->AddSeparatorItem();
	m_menu->AddItem(submenu);
	// Le menu Bookmarks
	m_bookmarks = new BMenu("Bookmarks");
	m_bookmarks->AddItem(menuitem=new BMenuItem("Add to Bookmarks",new BMessage(bmsgBookmarksAdd),'B',B_COMMAND_KEY));
	menuitem->SetEnabled(false);
	m_bookmarks->AddItem(menuitem=new BMenuItem("Show Bookmarks",new BMessage(bmsgBookmarksShow)));
	menuitem->SetEnabled(false);
	m_bookmarks->AddSeparatorItem();
	
#ifdef __BEOS__
	// Read bookmarks
	BDirectory dir("/boot/home/config/settings/NetPositive/Bookmarks");
	int32 count = dir.CountEntries();
	BEntry entry;
	
	for(int32 i=0; i<count; i++) {
		if(B_ENTRY_NOT_FOUND == dir.GetNextEntry(&entry, true))
			break;

		BNode node(&entry);
		char url[256];		
		char name[256];
		if(0 < node.ReadAttr("META:url", B_STRING_TYPE, 0, url, 256/*sizeof url*/)
			&& 0 < node.ReadAttr("META:title", B_STRING_TYPE, 0, name, 256/*sizeof name*/)) {
			BMessage *msg = new BMessage(bmsgBookmarkSelected);
			msg->AddString("url", url);
			m_bookmarks->AddItem(new BMenuItem(name, msg));
		}
	}	

	m_bookmarks->AddSeparatorItem();
	// Add Bookmarks submenu
	m_menu->AddItem(m_bookmarks);
#endif
	
	// Le menu View
	submenu = new BMenu("View");
//	submenu->SetEnabled(false);
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
	submenu1->SetEnabled(false);
	submenu1->AddItem(menuitem=new BMenuItem("Auto Detect", new BMessage(bmsgViewEncAuto)));
	submenu1->AddSeparatorItem();
	submenu1->AddItem(menuitem=new BMenuItem("Central European (ISO 8859-2)", new BMessage(bmsgViewEncISO88592)));
	submenu1->AddItem(menuitem=new BMenuItem("Cyrillic (ISO 8859-5)", new BMessage(bmsgViewEncISO88595)));
	submenu1->AddItem(menuitem=new BMenuItem("Cyrillic (KOI8-R)", new BMessage(bmsgViewEncKOI8R)));
	submenu1->AddItem(menuitem=new BMenuItem("Cyrillic (MS-DOS 866)", new BMessage(bmsgViewEncMSDOS866)));
	submenu1->AddItem(menuitem=new BMenuItem("Cyrillic (Windows 1251)", new BMessage(bmsgViewEncWin1251)));
	submenu1->AddItem(menuitem=new BMenuItem("Greek (ISO 8859-7)", new BMessage(bmsgViewEncISO88597)));
	submenu1->AddItem(menuitem=new BMenuItem("Japanese (Auto Detect)", new BMessage(bmsgViewEncJAuto)));
	submenu1->AddItem(menuitem=new BMenuItem("Japanese (Shift-JIS)", new BMessage(bmsgViewEncJIS)));
	submenu1->AddItem(menuitem=new BMenuItem("Japanese (EUC)", new BMessage(bmsgViewEncEUC)));
	submenu1->AddItem(menuitem=new BMenuItem("Unicode", new BMessage(bmsgViewEncUnicode)));
	submenu1->AddItem(menuitem=new BMenuItem("Unicode (UTF-8)", new BMessage(bmsgViewEncUTF8)));
	submenu1->AddItem(menuitem=new BMenuItem("Western (ISO 8859-1)", new BMessage(bmsgViewEncISO88591)));
	submenu1->AddItem(menuitem=new BMenuItem("Western (Mac Roman)", new BMessage(bmsgViewEncMac)));
	
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
	
#ifdef __BEOS__
#if 1
	urlControl = new BTextControl(BRect(30,5,250,20), "url view",NULL, "http://", new BMessage(URL_ENTERED));
	//urlControl->SetDivider(0);
	urlControl->SetTarget(this);
#else
	urlControl = new BTextView(BRect(3,5,250,20), "url view",BRect(3,5,250,20), B_FOLLOW_TOP | B_FOLLOW_LEFT, B_WILL_DRAW);
#endif
	toolBarView->AddChild(urlControl);
#endif
	
	int const LinkBarHeight = 15;
	
	r = Bounds();
	r.top+=toolBarView->Frame().bottom+3;
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.bottom -= B_H_SCROLL_BAR_HEIGHT * 2 + LinkBarHeight;

	drawArea = new HTMLView (this, r);
#ifdef __BEOS__
	scrollView = new BScrollView("HtmlScroll", drawArea, B_FOLLOW_ALL_SIDES, B_NAVIGABLE, true, true);
	AddChild(scrollView);
#else
	AddChild(drawArea);
#endif
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
#ifdef __BEOS__
			fprintf(stderr, "User asks %s\n", urlControl->Text());
			if (Lock()) {
				drawArea->SetUrl(urlControl->Text());
				Unlock();
				UpdateNavControls();
			}
#endif
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
				bool reformat = false;
				message->FindBool("reformat", &reformat);
				if (reformat)
					drawArea->Redraw();
				else
					drawArea->Refresh();
			}
			break;
		case bmsgButtonBACK:
			if (!History::history.Back()) {
				fprintf(stdout, "######## No history to go back\n");
				return;
			}
			else				
				fprintf(stdout, "######## Going back\n");
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
		case bmsgBookmarkSelected: {
			char *url = new char[256];
			fprintf(stdout, "*** Got BookmarkSelected message\n");
			if (B_OK == message->FindString("url", (const char **)&url)) {
				fprintf(stdout, "*** URL is %s\n", url);
				SetUrl(url);
				UpdateNavControls();
			}
			//delete url;
			break;
		}
#ifdef __BEOS__
		case bmsgViewReload:
		case bmsgButtonRELOAD:
			Cache::cache.ClearAll();
			SetUrl(this->urlControl->Text());
			break;

		case bmsgEditPreferences: {
			PrefWindow *wnd = new PrefWindow();
			wnd->Show();
			break;
		}
#endif
		case bmsgViewFullscreen: {
			FullScreen();
			break;
		}	
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
				fprintf(stderr, "Message received %4s in NO Window\n", (char *)&message->what);
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
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}

