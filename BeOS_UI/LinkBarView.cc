#include "LinkBarView.h"
#include "Document.h"
#include "UIMessages.h"
#include <Font.h>
#include <Message.h>
#include <Window.h>

void LinkBarView::AddLinkFromDocument(const Document *docInfo) {
	m_docInfo = docInfo;
}

void LinkBarView::Draw(BRect /*update*/) {
	if (!m_docInfo) return;
	BFont f;
	GetFont(&f);
	BPoint point;
	point.y = Bounds().bottom - 3;
	point.x = 3;
	StrRef name;
	StrRef url;
	for (int i=0; m_docInfo->GetLinkAt(i, &name, &url); i++) {
		if (url.IsFree()) continue;
		const char *linkName = name.Str();
		DrawString(linkName, point);
		point.x += f.StringWidth(linkName) + 5;
	}
}

void LinkBarView::MouseDown(BPoint where) {
	int x = 3;
	BFont f;
	GetFont(&f);
	StrRef name;
	StrRef url;
	for (int i=0; where.x > x && m_docInfo->GetLinkAt(i, &name, &url); i++) {
		if (url.IsFree()) continue;
		const char *linkName = name.Str();
		x += f.StringWidth(linkName) + 5;
	}
	if (!url.IsFree() && where.x<x) {
		BWindow *win = Window();
		if (win) {
			BMessage *m = new BMessage(bmsgBookmarkSelected);
			m->AddString("url", url.Str());
			win->PostMessage(m);
		}
	}
}
