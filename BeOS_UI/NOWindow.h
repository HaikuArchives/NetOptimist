#ifndef NOWindow_H
#define NOWindow_H
#include <Window.h>
#include "Html.h"
#include "HTMLView.h"
#include "ToolBarView.h"

class BTextControl;
class BMenu;
class BMenuItem;
class LinkBarView;
class StatusBarView;

class NOWindow : public BWindow {
	const int ToolBarViewHeigth;
	bool m_fullScreen;
	BRect m_orgFrame; // the frame before Full Screen
		
	DocFormater *format;
	ToolBarView *toolBarView;
	BTextControl	*urlControl;
	HTMLView *drawArea;
	BScrollView *scrollView;
	StatusBarView *m_statusView;
	LinkBarView * m_linkBarView;

	BMenuBar *m_menu;
	
//	BMenuItem *m_goBack;
//	BMenuItem *m_goForward;	
	BMenu *m_bookmarks;
	
	BMenuItem *m_debugForceTableBorder;
	BMenuItem *m_debugExportFWTFile;
	BMenuItem *m_debugNoFillRect;
public :
	
	NOWindow(BRect windowfr); 
	void SetUrl(const char *url);
	bool QuitRequested();
	void MessageReceived(BMessage *message);
private:
	void UrlChanged(const char *);
	void UpdateNavControls();
	void FullScreen();
};

#endif
