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
	m_includedStyle->SetFace(B_ITALIC_FACE);
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
        	m_includedStyle->SetFace(B_BOLD_FACE);
        	m_includedStyle->SetSize(24);
}

void H2_DocElem::geometry(HTMLFrame *view) {
        	TagDocElem::geometry(view);
	m_includedStyle = m_style->clone(id);
        	m_includedStyle->SetFace(B_BOLD_FACE);
        	m_includedStyle->SetSize(18);
}

void H3_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
	m_includedStyle->SetFace(B_BOLD_FACE);
	m_includedStyle->SetSize(14);
}

void H4_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
        	m_includedStyle->SetFace(B_BOLD_FACE);
        	m_includedStyle->SetSize(12);
}

void H5_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
        	m_includedStyle->SetFace(B_BOLD_FACE);
        	m_includedStyle->SetSize(11);
}

void H6_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
       	m_includedStyle->SetFace(B_BOLD_FACE);
        	m_includedStyle->SetSize(10);
}

void BLOCKQUOTE_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
       	m_includedStyle->SetFace(B_ITALIC_FACE);
}

void BIG_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
	m_includedStyle->SetSize(14);
}

void SMALL_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
	m_includedStyle->SetSize(8);
}

void FIXED_DocElem::geometry(HTMLFrame *view) {
        TagDocElem::geometry(view);
        m_includedStyle = m_style->clone(id);
/*
#ifndef __BEOS__
        m_includedStyle->SetFont(be_fixed_font);
#else
        m_includedStyle->SetFont(be_bold_font);
#endif
*/
}

