#ifndef HTMLView_H
#define HTMLView_H

#include <View.h>
#include <Message.h>
#include <ScrollView.h>
#include <Font.h>
#include "Frame.h"

class Style;
class StatusBarView;

class HTMLView : public BView, public HTMLFrame
{
	BScrollView *scrollView;
	StatusBarView *m_statusBar;
	uint32 sourceEncoding_;
	BFont propFont_;
	int32 propFontSize_;
	
public :
	HTMLView(BLooper *container, BRect fr);
	
	void Draw(BRect updateRect);
	void MouseDown(BPoint point);
	virtual void MouseMoved( BPoint where, uint32 code, const BMessage *a_message);
	void Pulse();
	void MessageReceived(BMessage *message);
	void KeyUp(const char *bytes, int32 numBytes);
	void KeyDown(const char *bytes, int32 numBytes);
	virtual void AttachedToWindow();
	virtual void FrameResized(float width, float height);
	void TargetedByScrollView(BScrollView *scroller);
	void SetStatusBarView(StatusBarView *);
	virtual void Message(const char *msg, msg_level_t level = LVL_MSG);
	
	void DrawRect(int x, int y, int w, int h, const Style *style, int penWidth = 1);
	void FillRect(int x, int y, int w, int h, const Style *style);
	void DrawBorder3D(int x, int y, int w, int h, const Style *style, int penWidth = 1, Look look = LOOK_UP);
	void DrawBadImg(int x, int y, int w, int h, const Style *style);
	void DrawImg(int x, int y, int w, int h, BBitmap *bmp);
	void StringDim(const char* str, const Style *style, int* w, int *h);
	void DrawString(int x, int y, int w, const char *str, const Style *style);
	void FillCircle(int a, int b, int d, const Style *style);
	void SetFrameColor(const Style *style);
	void SetFrame(int maxX, int maxY);
	void SetTitle(const char *title);
	virtual void IncResourceWaiting();
	virtual void DecResourceWaiting();
	virtual void NewDocumentLoaded(void);
	void Refresh();
	void Redraw();

	uint32 SourceEncoding();
	void SetSourceEncoding(uint32);
};
#endif
