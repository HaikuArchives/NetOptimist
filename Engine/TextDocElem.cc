#include "TextDocElem.h"

void FONT_DocElem::geometry(HTMLFrame *view) { 
	rgb_color textColor;
	TagDocElem::geometry(view);

	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		if (iter->ReadColor("COLOR", &textColor)) {
			m_includedStyle = m_style->clone(id);
			m_includedStyle->SetColor(textColor);
		}
		iter->ReadSize("SIZE", &attr_size);
		iter->ReadStrl("FACE", attr_font, sizeof(attr_font));
	}
}

void B_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
        m_includedStyle->SetFace(B_BOLD_FACE);
}

void I_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
#ifndef __BEOS__
        m_includedStyle->SetFont(be_italic_font);
#else
//        m_includedStyle->SetFont(be_bold_font);
       m_includedStyle->SetFace(B_ITALIC_FACE);
#endif
}

void U_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
	m_includedStyle->SetUnderline();
       m_includedStyle->SetFace(B_UNDERSCORE_FACE);
}

void H1_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
#ifndef __BEOS__
        m_includedStyle->SetFont(be_big_font);
#else
        m_includedStyle->SetFont(be_bold_font);
#endif
}

void H2_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
#ifndef __BEOS__
        m_includedStyle->SetFont(be_big_font);
#else
        m_includedStyle->SetFont(be_bold_font);
#endif
}

void H3_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
        m_includedStyle->SetFont(be_bold_font);
}

void BLOCKQUOTE_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
#ifndef __BEOS__
        m_includedStyle->SetFont(be_italic_font);
#else
        m_includedStyle->SetFont(be_bold_font);
#endif
}

void BIG_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
#ifndef __BEOS__
        m_includedStyle->SetFont(be_big_font);
#else
        m_includedStyle->SetFont(be_bold_font);
#endif
}

void SMALL_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
#ifndef __BEOS__
        m_includedStyle->SetFont(be_small_font);
#else
        m_includedStyle->SetFont(be_bold_font);
#endif
}

void FIXED_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
#ifndef __BEOS__
        m_includedStyle->SetFont(be_fixed_font);
#else
        m_includedStyle->SetFont(be_bold_font);
#endif
}

