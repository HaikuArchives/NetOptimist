#ifndef ToolBarView_H
#define ToolBarView_H

class BList;
class BLooper;
class BAlignment;
#include <View.h>

class ToolBarView : public BView 
{
private:
	BList *toolList;
	BBitmap *bg1,*bg12,*bg2;
	const BBitmap *m_documentIcon;
public :
	BBitmap *bitmap; 
	ToolBarView(BRect r, BLooper *looper);
	~ToolBarView();
	virtual void FrameResized(float w,float h);
	void AttachedToWindow();
	void Draw(BRect updateRect);
	void MessageReceived(BMessage *message);
	
	void computeBitmap(BRect r);
	void SetDocumentIcon(const BBitmap *bmp);
	
	bool SetEnabled(uint32 cmd, bool newState);
};

#endif
