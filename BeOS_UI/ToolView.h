#ifndef ToolView_h
#define ToolView_h
#include <Bitmap.h>
#include <View.h>

class ToolView : public BView
{
	bool m_enabled;
public:
	
	BBitmap *bitmapLow;
	BBitmap *bitmapHigh;
	BBitmap *bitmapDisabled;
	BMessage *invok_message;
	bool click;
	bool original;
	
	ToolView(BRect r, const char *a_name, BMessage *msg, BBitmap *low, BBitmap *high, BBitmap *disabled = NULL);
	ToolView(BMessage *data);
	~ToolView();
	void AttachedToWindow();
	virtual status_t Archive(BMessage *data, bool deep) const;
	static ToolView *Instantiate(BMessage *data);
	
	void MouseDown(BPoint point); 
	void MouseMoved(BPoint point,uint32 transit, const BMessage *message);
	void MouseUp(BPoint point); 
	void Draw(BRect r);
	bool SetEnabled(bool);
};
#endif
