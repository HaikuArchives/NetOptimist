#ifndef LinkBarView_H
#define LinkBarView_H

#include <View.h>

class Document;

class LinkBarView : public BView {
	const Document *m_docInfo;
public:
	LinkBarView(BRect frame, const char *name,
		uint32 resizeMask, uint32 flags) :
		BView(frame, name, resizeMask, flags | B_WILL_DRAW)
	{
		m_docInfo = NULL;
	}
	void AddLinkFromDocument(const Document *docInfo); 
	virtual void Draw(BRect updateRect);
};

#endif
