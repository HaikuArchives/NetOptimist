#include "ToolBarView.h"
#include "Pref.h"
#include "traces.h"
#include <File.h>
#include <Alert.h>
#include <Resources.h>
#include <Message.h>
#include <Bitmap.h>
#include <stdio.h>
#include "ToolView.h"
#include "UIMessages.h"
#include <TranslationKit.h>
#include <Dragger.h>

#include <iostream.h>

#if B_HOST_IS_LENDIAN
#define ARGB_FORMAT B_RGBA32_LITTLE
struct ARGBPixel { uint8 b,g,r,a; };
#else
#define ARGB_FORMAT B_RGBA32_BIG
struct ARGBPixel { uint8 a,r,g,b; };
#endif

ToolBarView::ToolBarView(BRect r, BLooper *l) :
	BView(r,"ToolBar", B_FOLLOW_LEFT_RIGHT, B_NAVIGABLE | B_FRAME_EVENTS | B_WILL_DRAW )
{
	m_documentIcon=NULL;
	bitmap=NULL;
	toolList=new BList();
	bg1=BTranslationUtils::GetBitmapFile("Icons/BackGround1.png");
	bg12=BTranslationUtils::GetBitmapFile("Icons/BackGround1_2.png");
	bg2=BTranslationUtils::GetBitmapFile("Icons/BackGround2.png");
	SetViewColor(B_TRANSPARENT_COLOR);
	computeBitmap(Bounds());

	/*
	BFile file("NetOptimist", B_READ_ONLY);
	if (file.InitCheck()!=B_OK) {
		(new BAlert("Error", "Cannot open resource file", "Ok"))->Go();
		return;
	}
	
	BResources res; 
	status_t err; 
	if ((err = res.SetTo(&file)) == B_OK) {
		size_t outSize;
		const void *data = res.LoadResource('MICN', 1, &outSize);
		if (!data) {
			//(new BAlert("Error", "Unable to load resources", "Ok"))->Go();
		} else {
			//docIcon = new BBitmap(BRect(0.0, 0.0, 15.0, 15.0), B_CMAP8);
			//memcpy(docIcon->Bits(), data, docIcon->BitsLength());
		}
	}
	//SetDrawingMode(B_OP_OVER); // This is needed to support transparent gifs
	*/

	ToolView *t;
	BRect r=Bounds();
	BRect r2=BRect(25,25,32,32) ; //BRect du dragger
	
	r.left=260;
	r.right=r.left+32;
	r.bottom-=2;

	BMessage *msg;
	BDragger *dragger;
	char bitmapLow[512];
	char bitmapHigh[512];
	
	// Back
	msg = new BMessage(bmsgButtonBACK);
	sprintf(bitmapLow,"%s/%s", Pref::Default.AppDir(), "Icons/BackLow.png");
	sprintf(bitmapHigh,"%s/%s", Pref::Default.AppDir(), "Icons/BackHigh.png");
	t=new ToolView(r, "Back",msg, BTranslationUtils::GetBitmapFile(bitmapLow),BTranslationUtils::GetBitmapFile(bitmapHigh));
	if(t)
	{
		toolList->AddItem(t);
		AddChild(t);	
		dragger= new BDragger(r2, t,0);
		if (dragger) t->AddChild(dragger);
	}
	// Next
	msg = new BMessage(bmsgButtonNEXT);
	r.left+=32;
	r.right+=32;
	sprintf(bitmapLow,"%s/%s", Pref::Default.AppDir(), "Icons/NextLow.png");
	sprintf(bitmapHigh,"%s/%s", Pref::Default.AppDir(), "Icons/NextHigh.png");
	t=new ToolView(r, "Next",msg, BTranslationUtils::GetBitmapFile(bitmapLow),BTranslationUtils::GetBitmapFile(bitmapHigh));
	if(t)
	{
		toolList->AddItem(t);
		AddChild(t);	
		dragger= new BDragger(r2, t,0);
		if (dragger) 
			t->AddChild(dragger);
		
	}
	// Stop
	msg= new BMessage(bmsgButtonSTOP);
	r.left+=32;
	r.right+=32;
	sprintf(bitmapLow,"%s/%s", Pref::Default.AppDir(), "Icons/StopLow.png");
	sprintf(bitmapHigh,"%s/%s", Pref::Default.AppDir(), "Icons/StopHigh.png");
	t=new ToolView(r, "Stop",msg, BTranslationUtils::GetBitmapFile(bitmapLow),BTranslationUtils::GetBitmapFile(bitmapHigh));
	if(t)
	{
		t->SetEnabled(false);	// XXX not implemented yet
		toolList->AddItem(t);
		AddChild(t);		
		dragger= new BDragger(r2, t,0);
		if (dragger) t->AddChild(dragger);
	}
	// Reload
	msg= new BMessage(bmsgButtonRELOAD);
	r.left+=32;
	r.right+=32;
	sprintf(bitmapLow,"%s/%s", Pref::Default.AppDir(), "Icons/ReloadLow.png");
	sprintf(bitmapHigh,"%s/%s", Pref::Default.AppDir(), "Icons/ReloadHigh.png");
	t=new ToolView(r, "Reload",msg, BTranslationUtils::GetBitmapFile(bitmapLow),BTranslationUtils::GetBitmapFile(bitmapHigh),BTranslationUtils::GetBitmapFile("Icons/ReloadDisabled.png"));
	if(t)
	{
		//t->SetEnabled(false);
		toolList->AddItem(t);
		AddChild(t);		
		dragger= new BDragger(r2, t,0);
		if (dragger) t->AddChild(dragger);
	}
	// Home
	msg= new BMessage(bmsgButtonHOME);
	r.left+=32;
	r.right+=32;
	sprintf(bitmapLow,"%s/%s", Pref::Default.AppDir(), "Icons/HomeLow.png");
	sprintf(bitmapHigh,"%s/%s", Pref::Default.AppDir(), "Icons/HomeHigh.png");
	t=new ToolView(r, "Home",msg, BTranslationUtils::GetBitmapFile(bitmapLow),BTranslationUtils::GetBitmapFile(bitmapHigh));
	if(t)
	{
		toolList->AddItem(t);
		AddChild(t);		
		dragger= new BDragger(r2, t,0);
		if (dragger) t->AddChild(dragger);
	}
	// Save
	msg= new BMessage(bmsgButtonSAVE);
	r.left+=32;
	r.right+=32;
	sprintf(bitmapLow,"%s/%s", Pref::Default.AppDir(), "Icons/SaveLow.png");
	sprintf(bitmapHigh,"%s/%s", Pref::Default.AppDir(), "Icons/SaveHigh.png");
	t=new ToolView(r, "Save", msg, BTranslationUtils::GetBitmapFile(bitmapLow),BTranslationUtils::GetBitmapFile(bitmapHigh), BTranslationUtils::GetBitmapFile("Icons/SaveDisabled.png"));
	if(t)
	{
		t->SetEnabled(false);	// XXX not implemented yet
		toolList->AddItem(t);
		AddChild(t);		
		dragger= new BDragger(r2, t,0);
		if (dragger) t->AddChild(dragger);
	}

	
}

ToolBarView::~ToolBarView() 
{
	//if (docIcon)
	//	delete docIcon;
	if (bitmap) {
		delete bitmap;
	}
}

void ToolBarView::FrameResized(float w, float h)
{
	computeBitmap(BRect(0,0,w,h));
	for(int i=0;i<toolList->CountItems();i++)
		((ToolView *)toolList->ItemAt(i))->Draw(((ToolView *)toolList->ItemAt(i))->Bounds());
	
	
}
void ToolBarView::computeBitmap(BRect r)
{
	if(bitmap!=NULL)
	{
		bitmap->RemoveChild(bitmapView);
		delete bitmap;
		delete bitmapView;
	}
	bitmap= new BBitmap(r,ARGB_FORMAT,true);
	bitmapView=new BView(r,"",B_FOLLOW_ALL,B_WILL_DRAW);
	bitmap->AddChild(bitmapView);
	bitmapView->LockLooper();
	int toolWidth;
	int totalWidth=(int)r.Width();
	
	if (totalWidth>400)
		toolWidth=totalWidth-200;
	else
		toolWidth=200;	

	if (bg1 && bg2 && bg12) {
		int bg1Width=(int)bg1->Bounds().Width();
		int bg12Width=(int)bg12->Bounds().Width();
		int bg2Width=(int)bg2->Bounds().Width();
		int x=0;
		while(x<toolWidth)
		{
			bitmapView->DrawBitmapAsync(bg1,BPoint(x,0));
			x+=bg1Width;
		}
		bitmapView->DrawBitmapAsync(bg12,BPoint(x,0));
		x+=bg12Width;
		while(x<totalWidth)
		{
			bitmapView->DrawBitmapAsync(bg2,BPoint(x,0));
			x+=bg2Width;
		}
		bitmapView->Sync();
		bitmapView->UnlockLooper();
	}	
	Draw(Bounds());
}


	
void ToolBarView::Draw(BRect updateRect) {
	DrawBitmap(bitmap, BPoint(0,0));
	if (m_documentIcon)
		DrawBitmap(m_documentIcon, BPoint(3,5));
}

void ToolBarView::MessageReceived(BMessage *message) {
	BView::MessageReceived(message);
	trace(DEBUG_MESSAGING)
		fprintf(stderr, "Message received %4s in ToolBarView\n", (char *)&message->what);
}

void ToolBarView::SetDocumentIcon(const BBitmap *bmp) {
	this->m_documentIcon = bmp;
	Invalidate();
}

bool ToolBarView::SetEnabled(uint32 cmd, bool newState) {
	if (!toolList) return false;
	ToolView *view = NULL;
	for (int32 i = 0; NULL != (view = (ToolView *)toolList->ItemAt(i)); i++) {
		if (NULL != view->message && cmd == view->message->what) {
			return view->SetEnabled(newState);
		} 	
	}
	return false;
}
