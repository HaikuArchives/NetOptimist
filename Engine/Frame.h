#ifndef FRAME_H
#define FRAME_H


class Style;
class BBitmap;
class Url;
class UrlQuery;
class ResourceGetter;
class DocFormater;
class BLooper;
class HTMLWindow;
#include <string.h>
#ifdef __BEOS__
#include <be/support/SupportDefs.h>
#endif
#include "User.h"
#include "Document.h"
#include "Action.h"



class HTMLFrame : public User {
	void SetToUrl(Url *newUrl);

	/* Html view geometry */
	int m_winw ;
	int m_winh ;
	UrlQuery *m_nextQuery;
	Url *m_nextUrl;	// This is the Url that the user asked for.
			// Not the current Url. If NULL, no url is
			// currently downloaded.
protected:
	DocFormater *m_format;
	ResourceGetter *m_getter;
	BLooper *m_container;
	void ModifyUrl(Url *newUrl);
public:
	Document m_document;
	enum Look { LOOK_UP, LOOK_DOWN } ;

	HTMLFrame(BLooper *container, int w, int h);
	virtual ~HTMLFrame();

	int DocWidth() {  return m_winw; }
	int DocHeight() { return m_winh; }
	virtual bool Resize(int w, int h);

	// Actions
	void SetUrl(const char *url_text);
	void SetUrl(UrlQuery *url_text);
	bool Select(int viewX, int viewY, Action);
	void CheckUrl();


	/* History */
	void Back();
	void Forward();

	/* These are hooks for platform specific code */
	// Frame
	virtual void SetFrame(int maxX, int maxY) = 0;
	virtual void SetFrameColor(const Style *style) = 0;
	virtual HTMLWindow* GetHTMLWindow() = 0;

	// Basic drawing functions
	virtual void DrawBorder3D(int x, int y, int w, int h, const Style *style, int penWidth = 1, Look look = LOOK_UP) = 0;
	virtual void DrawRect(int x, int y, int w, int h, const Style *style, int penWidth = 1) = 0;
	virtual void FillRect(int x, int y, int w, int h, const Style *style) = 0;
	virtual void DrawImg(int x, int y, int w, int h, BBitmap *bmp) = 0;
	virtual void DrawBadImg(int x, int y, int w, int h, const Style *style) = 0;
	virtual void StringDim(const char* str, const Style *style, int* w, int *h) = 0;
	virtual void DrawString(int x, int y, int w, const char *str, const Style *style) = 0;
	virtual void FillCircle(int a, int b, int d, const Style *style) = 0;
	// General drawing functions
	virtual void Redraw(void)=0;
	virtual void Refresh(void)=0;
	virtual void NewDocumentLoaded(void)=0;
	// Use these methods to tell the user that a resource is being downloaded
	virtual void IncResourceWaiting() = 0;
	virtual void DecResourceWaiting() = 0;

	virtual uint32 SourceEncoding(void) const = 0;
	virtual void SetSourceEncoding(uint32) = 0;
	virtual char * Decode(const char *) = 0;

};

#endif
