#include "ToolView.h"
#include <TranslationKit.h>
#include <iostream.h>
#include <Roster.h>
#include <Alert.h>
#include <Entry.h>
#include <Application.h>
#include <stdio.h>
#include "ToolBarView.h"

#include "platform.h"


ToolView::	ToolView(BRect r, const char *a_name, BMessage *msg, BBitmap *low, BBitmap *high, BBitmap *disabled)
:BView(r, a_name , B_FOLLOW_NONE, B_FRAME_EVENTS|B_WILL_DRAW|B_NAVIGABLE)
{
	message=msg;
	
	bitmapLow=low;
	bitmapHigh=high;
	if (disabled)
		bitmapDisabled=disabled;
	else
		bitmapDisabled=low;
	
	
	click=false;
	//SetViewColor(B_TRANSPARENT_COLOR);
	original=true;
	m_enabled = true;
	
}
void ToolView::AttachedToWindow()
{
	//SetViewBitmap(BTranslationUtils::GetBitmapFile("Icons/BackGround1.png"));
}
ToolView::ToolView(BMessage *data) : BView(data)
{
	BAlert *alert = new BAlert(NULL, "Restauration du bitmapLow impossible!", "OK");
	alert->Go();
	
	BMessage restore;
	if(data->FindMessage("bitmapLow", &restore) == B_OK)
	{
		bitmapLow = (BBitmap *)instantiate_object(&restore);
	}
	else
	{
		BAlert *alert = new BAlert(NULL, "Restauration du bitmapLow impossible!", "OK");
		alert->Go();
	}
	
	if(data->FindMessage("bitmapHigh", &restore) == B_OK)
		bitmapHigh = (BBitmap *)instantiate_object(&restore);
	else
	{
		BAlert *alert = new BAlert(NULL, "Restauration du bitmapHigh impossible!", "OK");
		alert->Go();
	}
	
	m_enabled = true;
	data->FindBool("original", &m_enabled);

	if(data->FindBool("original", &original) == B_OK)
		original=false;
	else 
		original=true;
		
	BMessage r;	
	if(data->FindMessage("my_message", &r) == B_OK)
	{
		//message = (BMessage *)instantiate_object(&restore);
	}
	else
	{
		BAlert *alert = new BAlert(NULL, "Restauration du message impossible!", "OK");
		alert->Go();
	}
	
	message = new BMessage(r);
	if(message==NULL)
	{
		BAlert *alert = new BAlert(NULL, "Message Null lors de la restauration!", "OK");
		alert->Go();
	}
	click=false;
	
}


ToolView::~ToolView()
{
	delete message;
}	

status_t ToolView::Archive(BMessage *data, bool deep) const
{
	
	BView::Archive(data, deep);
	BMessage archive1,archive2;
	BMessage me(*message);
	bitmapLow->Archive(&archive1);
	data->AddMessage("bitmapLow", &archive1);
	
	bitmapHigh->Archive(&archive2);
	data->AddMessage("bitmapHigh", &archive2);
	
	data->AddString("add_on", app_signature);
	data->AddString("class", "ToolView");
	data->AddRect("bounds", Bounds());
	data->AddBool("enabled",true);
	data->AddBool("original",false);
	cout <<"Contenu du message archive\n";
	me.PrintToStream();
	data->AddMessage("my_message", &me);
	return (B_NO_ERROR);
}

ToolView* ToolView::Instantiate(BMessage *data)
{
	if(validate_instantiation(data, "ToolView"))
		return new ToolView(data);
	return NULL;
}



void ToolView::MouseDown(BPoint point)
{ 
	if(message==NULL)
	{
		BAlert *alert = new BAlert(NULL, "Message Null!", "OK");
		alert->Go();
	}
	else if (m_enabled)
	{
		if (be_app->PostMessage(message) != B_OK) {
			BAlert *alert = new BAlert(NULL, "Could not send message to application", "OK");
			alert->Go();
		}
	}
	
	click=true;
	Draw(Bounds());
}	

void ToolView::MouseMoved(BPoint point,uint32 transit, const BMessage *message)
{ 
	if (transit==B_EXITED_VIEW || transit==B_ENTERED_VIEW )
	{	
		MouseUp(point);
	}

}
void ToolView::MouseUp(BPoint point)
{
	click=false;
	Draw(Bounds());
}

void ToolView::Draw(BRect r)
{
	
	SetDrawingMode(B_OP_COPY);
	//SetHighColor((rgb_color){44,55,44,124});
	if(!original)
	{
		SetHighColor(Parent()->ViewColor());
		//SetHighColor(ui_color(B_DESKTOP_COLOR));
		FillRect(Bounds());
	}
	else
		DrawBitmap( ( (ToolBarView *)Parent() )->bitmap, Frame(), Bounds());
	
	SetBlendingMode(B_PIXEL_ALPHA,B_ALPHA_OVERLAY);
	SetDrawingMode(B_OP_ALPHA);
	if(bitmapLow!=NULL && bitmapHigh!=NULL)
	{
		if (!m_enabled)
			DrawBitmap( bitmapDisabled, BPoint(0,0));
		else if (click)
			DrawBitmap( bitmapLow, BPoint(0,0));
		else
			DrawBitmap( bitmapHigh, BPoint(0,0));
	}
	SetDrawingMode(B_OP_COPY);
	//printf("llllll\n");
}

bool ToolView::SetEnabled(bool enabled)
{
	bool prevState = m_enabled;
	m_enabled = enabled;
	if (prevState != m_enabled) Invalidate();
	return prevState;
}
