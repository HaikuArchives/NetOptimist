#ifndef Style_H
#define Style_H

#include "platform.h"
#include <stdio.h>
#include <string.h>
#include "Colors.h"
#include "Alignment.h"

#include <Font.h>

class Style {
	int ownerId;
	rgb_color color;
	rgb_color bgcolor;
	rgb_color linkcolor;
	short m_indent;
	bool m_nobr;
	bool isLink;
	/* Text attributes */
	bool isUnderline;
	uint16 face_;
	int size_;
	Alignment m_alignment;

	int m_marginTop;
	int m_marginBottom;
	int m_marginLeft;
	int m_marginRight;
	
	Style(const Style &c);
public:
	Style();
	rgb_color Color() const {
#ifdef DEBUG_COLOR
		if (!memcmp(&color,&bgcolor,sizeof color)) {
			fprintf(stderr, "Warning, color & bg color identical\n");
			if (memcmp(&kWhite,&bgcolor,sizeof color))
				return kWhite;
			else
				return kBlack;
		} else
#endif

		return color;
	}
	rgb_color BGColor() const { return bgcolor; }
	rgb_color LinkColor() const { return linkcolor; }
	void SetColor(const rgb_color c) { color = c; }
	void SetBGColor(const rgb_color c) { bgcolor = c; }
	void SetLinkColor(const rgb_color c) { linkcolor = c; }
	virtual Style *clone(int owner) const {
		Style *s = new Style(*this);
		s->ownerId = owner;
		return s;
	}
	
//	void SetFontFamily(font_family f) { if (font) font->SetFamilyAndStyle(f, NULL); }

	// Font attributes access
	void SetFace(uint16 f);
	uint16 Face() const { return face_; }	
	void SetSize(int s) { size_ = s; }
	int Size() const { return size_; }
	
	void SetAlignment(Alignment);
	Alignment Align() const;
	const char *toString() const;
	void SetLink();
	bool IsLink() const;
	void SetUnderline(); // obsolete
	bool IsUnderline() const;
	void IncIndent(short inc);
	short Indent() const;
	int OwnerId() const { return ownerId; }
};


class StyleMgr {
	Style *defaultStyle;
	StyleMgr () {
		defaultStyle = new Style;
	}
	~StyleMgr () {
		delete defaultStyle;
	}
public:
	static const StyleMgr DefaultStyleMgr;
	const Style *DefaultStyle() const { return defaultStyle; }
	Style *cloneStyle(Style *style, int ownerId) {
		return style->clone(ownerId);
	}
};

#endif
