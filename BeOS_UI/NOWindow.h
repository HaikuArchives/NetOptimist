#ifndef NOWindow_H
#define NOWindow_H
#include <Window.h>

class BTextControl;
class BTextView;
class BScrollView;
class BMenuItem;
class BMenuBar;

class LinkBarView;
class ToolBarView;
class StatusBarView;
class HTMLView;
class DocFormater;

class NOWindow : public BWindow {
	const int ToolBarViewHeigth;
	bool m_fullScreen;
	BRect m_orgFrame; // the frame before Full Screen
		
	DocFormater *format;
	ToolBarView *toolBarView;
	BTextView	*urlControl;
	HTMLView *drawArea;
	BScrollView *scrollView;
	StatusBarView *m_statusView;
	LinkBarView * m_linkBarView;

	BMenuBar *m_menu;
	
	BMenuItem *m_debugForceTableBorder;
	BMenuItem *m_debugExportFWTFile;
	BMenuItem *m_debugNoFillRect;
public :
	
	NOWindow(BRect windowfr); 
	void SetUrl(const char *url);
	bool QuitRequested();
	void MessageReceived(BMessage *message);
	void IsDownloading(bool);
private:
	void UrlChanged(const char *);
	void UpdateNavControls();
	void FullScreen();
	uint32 SetEncoding(uint32 encoding);
};


#endif
