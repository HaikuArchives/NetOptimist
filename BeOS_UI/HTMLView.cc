#include "HTMLView.h"

#include <Window.h>
#include <ScrollView.h>
#include <be/support/UTF8.h>
#include "NOWindow.h"
#include "Html.h"
#include "Style.h"
#include "StrPlus.h"
#include "StatusBarView.h"
#include "traces.h"
#include "Pref.h"

HTMLView::HTMLView(BLooper *container, BRect fr) : 
		BView(fr, "DrawArea", B_FOLLOW_ALL_SIDES, B_FRAME_EVENTS | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE/* | B_PULSE_NEEDED */),
		HTMLFrame(container, fr.Width(), fr.Height())
{
	sourceEncoding_ = Pref::Default.Encoding();
	SetSourceEncoding(sourceEncoding_);
	SetDrawingMode(B_OP_OVER); // This is needed to support transparent gifs
	scrollView = NULL;
}

void HTMLView::SetFrameColor(const Style *style) {
	if (ISTRACE(FILLRECT))
		SetViewColor(style->BGColor());
}

void HTMLView::AttachedToWindow() {
	//this->SetEventMask(B_POINTER_EVENTS); // this is to have soft MouseMoved events
}

void HTMLView::SetFrame(int maxX, int maxY) {
	BScrollBar *sb;

	sb = scrollView ? scrollView->ScrollBar(B_HORIZONTAL) : NULL;
	if (sb) {
		float w = this->Bounds().Width();
		if (maxX>w) {
			sb->Show();
			sb->SetRange(0, maxX-w);
			sb->SetSteps(w/10, w);
			sb->SetProportion(w/maxX);
		} else {
			sb->SetRange(0, 0);		// disable scrollBar
			// FIXME: we need to HIDE scrollers, not just disable them
		}
	}

	sb = scrollView ? scrollView->ScrollBar(B_VERTICAL) : NULL;
	if (sb) {
		float h = this->Bounds().Height();
		if (maxY>h) {
			sb->SetProportion(h/maxY);
			sb->SetRange(0, maxY-h);
			sb->SetSteps(h/10, h);
		} else {
			sb->SetRange(0, 0);		// disable scrollBar
			// FIXME: we need to HIDE scrollers, not just disable them
		}
	}
}

HTMLWindow *HTMLView::GetHTMLWindow() {
	NOWindow *win = dynamic_cast<NOWindow *>( Window() );
	return win;
}

void HTMLView::SetTitle(const char *title) {
	BWindow *window = Window();
	if (window) window->SetTitle(title);
}

void HTMLView::IncResourceWaiting() {
	NOWindow *container = dynamic_cast<NOWindow *>(Window());
	if (container) {
		container->IsDownloading(true);
	}
}

void HTMLView::DecResourceWaiting() {
	NOWindow *container = dynamic_cast<NOWindow *>(Window());
	if (container) {
		container->IsDownloading(false);
	}
}


void HTMLView::Draw(BRect updateRect) {
	if (m_format) {
		m_format->Draw(updateRect);
	}
}

void HTMLView::FrameResized(float width, float height) {
	if (Resize((int)width, (int)height)) {
		Redraw(); // FIXME: will redraw the whole view == BAD
	}
}

void HTMLView::Redraw() {
	// XXX bad : we should only reformat if something has changed in size
	if (m_format) {
		m_format->InvalidateLayout();
	}
	Invalidate();
}

void HTMLView::Refresh() {
	if (m_format) {
		m_format->Draw(Bounds(), true);
	}
}

void HTMLView::NewDocumentLoaded() {
	const char *title = m_document.TitleRef();
	if (strnull(title)) {
		fprintf(stderr, "HTMLView::SetTitle : title is null\n");
		title = "NetOptimist";
	}
	BWindow *window = Window();
	char *buf = Decode(title);
	if (window) window->SetTitle(buf);
	FREE(buf);
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
		fprintf(stderr, "Message received %4.4s in HTMLView\n", (char *)&message->what);
	BView::MessageReceived(message);
}

void HTMLView::KeyUp(const char *bytes, int32 numBytes) {
	if (ISTRACE(DEBUG_MESSAGING))
		fprintf(stderr, "KeyUp in HTMLView\n");
	if (scrollView)
		scrollView->KeyUp(bytes, numBytes);
}
	
void HTMLView::KeyDown(const char *bytes, int32 numBytes) {
	if (ISTRACE(DEBUG_MESSAGING))
		fprintf(stderr, "KeyDown in HTMLView\n");
	if (scrollView)
		scrollView->KeyDown(bytes, numBytes);
}

void HTMLView::TargetedByScrollView(BScrollView *scroller) {
	scrollView = scroller;
	BView::TargetedByScrollView(scroller);
}

void HTMLView::StringDim(const char* str, const Style *style, int* w, int *h) {

	PrepareFont(style);

	*w = (int) font_.StringWidth(str);
	font_height fh;
	font_.GetHeight(&fh);
	*h = fh.leading + fh.ascent + fh.descent; // XXX TODO : round it up
	if (style->IsUnderline())
		(*h)++;
}

void HTMLView::DrawString(int x, int y, int w, const char *str, const Style *style) {

	PrepareFont(style);

	SetHighColor(style->Color());
	SetLowColor(style->BGColor());

	BView::SetFont(&font_);

// NEXUS/FIXME: B_UNDERSCORE_FACE should do the trick, but doesn't do it for some reason...

	if (style->IsUnderline()) {
		StrokeLine(BPoint(x,y), BPoint(x+w,y));
		MovePenTo(x, y-1);
	} else {
		MovePenTo(x, y);
	}
	BView::DrawString(str);

}

void HTMLView::PrepareFont(const Style *style) {
	font_.SetEncoding(B_UNICODE_UTF8);
	font_.SetFamilyAndStyle(Pref::Default.FontFamily(displayEncoding_), NULL);
	// HACK : Depending on font chosen, B_UNDERSCORE_FACE is not supported
	font_.SetFace(style->Face()&~B_UNDERSCORE_FACE);
	if (0 < style->Size()) font_.SetSize(style->Size());
	else font_.SetSize(Pref::Default.FontSize(displayEncoding_));
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
		c2 = kDarkGray;
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

void HTMLView::SetSourceEncoding(uint32 enc) {
	sourceEncoding_ = enc;
	displayEncoding_ = NO_WESTERN;
	switch (sourceEncoding_) {
		case B_ISO5_CONVERSION:
		case B_KOI8R_CONVERSION:
		case B_MS_DOS_866_CONVERSION:
		case B_MS_WINDOWS_1251_CONVERSION:
			displayEncoding_ = NO_CYRILLIC;
			break;
		case B_ISO2_CONVERSION:
			displayEncoding_ = NO_CENTRAL_EUROPEAN;
			break;
		case B_ISO1_CONVERSION:
		case B_MAC_ROMAN_CONVERSION:
			displayEncoding_ = NO_WESTERN;
			break;
		case B_ISO7_CONVERSION:
			displayEncoding_ = NO_GREEK;
			break;
		case B_UNICODE_CONVERSION:
			displayEncoding_ = NO_UNICODE;
			break;
		case B_JIS_CONVERSION:
		case B_SJIS_CONVERSION:
		case B_EUC_CONVERSION:
			displayEncoding_ = NO_JAPANESE;
			break;
	}
}

uint32 HTMLView::SourceEncoding() const { return sourceEncoding_; }

// the caller should free up the memory referenced by the pointer returned
char * HTMLView::Decode(const char * str) {
	char *destBuf; 
	int32 destLen = strlen(str);
	if (0 != sourceEncoding_) {
		char *srcBuf = strdup(str);
		int32 srcLen = destLen;
		destLen = srcLen*4+1;
		int32 state = 0;
		destBuf = (char *) malloc(destLen); 
		memset(destBuf, 0, destLen);
		convert_to_utf8(sourceEncoding_, srcBuf, &srcLen, destBuf, &destLen, &state);
		FREE(srcBuf);
	} else destBuf = strdup(str);
	return destBuf;
}
