#if !defined(ENGINE_HTMLWINDOW_H)
#define ENGINE_HTMLWINDOW_H

class HTMLFrame;

class HTMLWindow {
public:
	virtual HTMLFrame *MainFrame() = 0;
};


#endif
