#include "LinkBarView.h"
#include "Document.h"
#include <stdio.h>
#include <Font.h>

void LinkBarView::AddLinkFromDocument(const Document *docInfo) {
	m_docInfo = docInfo;
}

void LinkBarView::Draw(BRect /*update*/) {
	if (!m_docInfo) return;
	BPoint point;
	point.y = Bounds().bottom - 3;
	point.x = 3;
	for (int i=0; ; i++) {
		const StrRef *ref = m_docInfo->GetLinkAt(i);
		if (!ref) break; // Exit from look
		if (ref->IsFree()) continue;
		const char *linkName = ref->Str();
		DrawString(linkName, point);
		BFont f;
		GetFont(&f);
		point.x += f.StringWidth(linkName) + 5;
	}
}
