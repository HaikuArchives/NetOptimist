#include "HTMLView.h"

#include <Window.h>
#include "Html.h"
#include "Style.h"
#include "StrPlus.h"
#include "StatusBarView.h"
#include "traces.h"

HTMLView::HTMLView(BLooper *container, BRect fr) : 
		BView(fr, "DrawArea", B_FOLLOW_ALL_SIDES, B_FRAME_EVENTS | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE/* | B_PULSE_NEEDED */),
		HTMLFrame(container, fr.Width(), fr.Height())
{
	SetDrawingMode(B_OP_OVER); // This is needed to support transparent gifs
	scrollView = NULL;
}

void HTMLView::SetFrameColor(const Style *style) {
	if (ISTRACE(FILLRECT))
		SetViewColor(style->BGColor());
}

void HTMLView::AttachedToWindow() {
	printf("attached to window\n");
	//this->SetEventMask(B_POINTER_EVENTS); // this is to have soft MouseMoved events
}

void HTMLView::SetFrame(int maxX, int maxY) {
	BScrollBar *sb;

	sb = scrollView->ScrollBar(B_HORIZONTAL);
	if (sb) {
		float w = this->Bounds().Width();
		if (maxX>w) {
			sb->SetRange(0, maxX-w);
			sb->SetSteps(w/10, w);
			sb->SetProportion(w/maxX);
		} else {
			sb->SetRange(0, 0);		// disable scrollBar
		}
	}

	sb = scrollView->ScrollBar(B_VERTICAL);
	if (sb) {
		float h = this->Bounds().Height();
		if (maxY>h) {
			sb->SetProportion(h/maxY);
			sb->SetRange(0, maxY-h);
			sb->SetSteps(h/10, h);
		} else {
			sb->SetRange(0, 0);		// disable scrollBar
		}
	}
}

void HTMLView::SetTitle(const char *title) {
	BWindow *window = Window();
	if (window) window->SetTitle(title);
}

void HTMLView::Draw(BRect updateRect) {
	if (m_format) {
		m_format->Msg(EXPOSE);
		m_format->ProcessAll();
	}
}

void HTMLView::FrameResized(float width, float height) {
	Resize((int)width, (int)height);
	Refresh();
}

void HTMLView::Refresh() {
	// XXX bad : we should only reformat if something has changed in size
	if (m_format) {
		m_format->Msg(FORMAT);
	}
	Invalidate();
}

void HTMLView::Redraw() {
	//Draw(Bounds());
	m_format->Msg(EXPOSE_IF_CHANGED);
	m_format->ProcessAll();
}

void HTMLView::NewDocumentLoaded() {
	if (strnull(m_document.TitleRef())) {
		fprintf(stderr, "HTMLView::SetTitle : title is null\n");
	} else {
		BWindow *window = Window();
		if (window) window->SetTitle(m_document.TitleRef());
	}
}

void HTMLView::MouseDown(BPoint point) {
	BView::MouseDown(point);
	Select(point.x, point.y, MOUSE_CLICK);
	MakeFocus();	// This will allow us to have mouse wheel events
}

void HTMLView::MouseMoved( BPoint where, uint32 code, const BMessage *a_message) {
	Select(where.x, where.y, MOUSE_OVER);
}

void HTMLView::MessageReceived(BMessage *message) {
	if (ISTRACE(DEBUG_MESSAGING))
		fprintf(stderr, "Message received %4s in HTMLView\n", (char *)&message->what);
	BView::MessageReceived(message);
}

void HTMLView::KeyUp(const char *bytes, int32 numBytes) {
	if (ISTRACE(DEBUG_MESSAGING))
		fprintf(stderr, "KeyUp in HTMLView\n");
	scrollView->KeyUp(bytes, numBytes);
	//BView::KeyUp(bytes, numBytes);
}
	
void HTMLView::KeyDown(const char *bytes, int32 numBytes) {
	if (ISTRACE(DEBUG_MESSAGING))
		fprintf(stderr, "KeyDown in HTMLView\n");
	scrollView->KeyDown(bytes, numBytes);
	//BView::KeyDown(bytes, numBytes);
}

void HTMLView::TargetedByScrollView(BScrollView *scroller) {
		scrollView = scroller;
		BView::TargetedByScrollView(scroller);
}

void HTMLView::StringDim(const char* str, const Style *style, int* w, int *h) {
	const BFont *f = (style && style->Font()) ? style->Font() : be_plain_font;
	*w = (int) f->StringWidth(str);
	font_height fh;
	f->GetHeight(&fh);
	*h = fh.leading + fh.ascent + fh.descent; // XXX TODO : round it up
	if (style->IsUnderline())
		(*h)++;
}

void HTMLView::DrawString(int x, int y, int w, const char *str, const Style *style) {
	if (style) {
		const BFont *f = (style->Font()) ? style->Font() : be_plain_font;
		BFont f2(f);
		f2.SetEncoding(B_ISO_8859_1);
		SetFont(&f2);
		SetHighColor(style->Color());
		SetLowColor(style->BGColor());
	}
	if (style->IsUnderline()) {
		MovePenTo(x, y-1);
		BView::DrawString(str);
		MovePenBy( 0, 1);
		StrokeLine(BPoint(x,y));
	} else {
		MovePenTo(x, y);
		BView::DrawString(str);
	}
}

void HTMLView::FillRect(int x, int y, int w, int h, const Style *style) {
	SetHighColor(style->BGColor());
	if (ISTRACE(FILLRECT))
		BView::FillRect(BRect(x, y, x+w-1, y+h-1));
}

void HTMLView::DrawRect(int x, int y, int w, int h, const Style *style, int penWidth) {
	SetHighColor(style->Color());
	StrokeRect(BRect(x, y, x+w-1, y+h-1));
}

void HTMLView::DrawBorder3D(int x, int y, int w, int h, const Style *style, int penWidth, Look look) {
	SetHighColor(style->BGColor());
	BView::FillRect(BRect(x, y, x+w-1, y+h-1));
	rgb_color c1, c2;
	if (look == LOOK_UP) {
		c1 = kLightGray;
		c2 = kGray;
	} else {
		c1 = kDarkGray;
		c2 = kLightGray;
	}
	for (int i=0; i<penWidth; i++) {
		SetHighColor(c1);
		StrokeLine(BPoint(x, y), BPoint(x+w-1, y));
		StrokeLine(BPoint(x, y), BPoint(x, y+h-1));
		SetHighColor(c2);
		StrokeLine(BPoint(x, y+h-1), BPoint(x+w-1, y+h-1));
		StrokeLine(BPoint(x+w-1, y), BPoint(x+w-1, y+h-1));
		x++; y++; w-=2; h-=2;
	}
}

void HTMLView::DrawBadImg(int x, int y, int w, int h, const Style *style) {
	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BView::FillRect(BRect(x, y, x+w-1, y+h-1));
}

void HTMLView::DrawImg(int x, int y, int w, int h, BBitmap *bmp) {
	if (bmp)
		BView::DrawBitmap(bmp, BRect(x, y, x+w-1, y+h-1));
	else
		BView::FillRect(BRect(x, y, x+w, y+h), B_MIXED_COLORS);
}

void HTMLView::FillCircle(int x, int y, int d, const Style *style) {
	SetHighColor(style->Color());
	BView::FillEllipse(BRect(x, y, x+d, y+d));
}

void HTMLView::Pulse() {
	printf("pulse\n");
}

void HTMLView::SetStatusBarView( StatusBarView *statusBarView) {
	m_statusBar = statusBarView;
}

void HTMLView::Message(const  char *msg, msg_level_t level) {
	m_statusBar->SetText(msg);
}
