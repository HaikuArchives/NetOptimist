#include "MetaDocElem.h"
#include "Style.h"
#include "UrlQuery.h"
#include "Url.h"
#include "Resource.h"
#include <Bitmap.h>

bool MetaDocElem::IsActive() {
	return !strnull(m_url.Str());
}

bool MetaDocElem::Action(::Action /*action*/, UrlQuery *href) {
	href->SetUrl(&m_url);
	return href->Url() != NULL;
}

void MetaDocElem::geometry(HTMLFrame *view) {
	StrRef httpEquivRef;
	StrRef contentRef;
	TagDocElem::geometry(view);
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		iter->ReadStrRef("CONTENT" ,&contentRef);
		iter->ReadStrRef("HTTP-EQUIV" ,&httpEquivRef);
	}
	if (!httpEquivRef.IsFree()) {
		printf("META : %s -> %s\n", httpEquivRef.Str(), contentRef.Str());
		if (!strcasecmp("refresh", httpEquivRef.Str())) {
			const char *content = contentRef.Str();
			while(content && *content) {
				int r = strcaseprefix(content, "url=");
				if (r>0) {
					printf("META URL=%s\n", content+r);
					m_url.SetToDup(content+r);
					break;
				}
				content++;
			}
		}
	}
	if (!m_url.IsFree()) {
		char *str = new char[strlen(m_url.Str()) + 30];
	
		sprintf(str, "[Redirect: %s]", m_url.Str());
		m_text.SetToString(str, false);
		view->StringDim(str, m_style,&w,&h);
		w+=3; // XXX Some spaces between words : bad AND USELESS ?
		fixedW = w;
	
		m_includedStyle = m_style->clone(id);
		m_includedStyle->SetColor(m_style->LinkColor());
		m_includedStyle->SetLink();
		m_includedStyle->SetUnderline();
	}
}

void MetaDocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	if (onlyIfChanged) return;
	if (!m_text.IsFree()) {
		view->DrawString(x,y+h-3,w-3,m_text.Str(),m_includedStyle); // XXX I don't really like the "-3" (see Also in "StrDocElem.cc")
	}
	printPosition("MetaDocElem::draw");
}

void BODY_DocElem::geometry(HTMLFrame *view) {
	TagDocElem::geometry(view);
	m_includedStyle = m_style->clone(id);
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		rgb_color color;
		if (iter->ReadColor("bgcolor",&color))
			m_includedStyle->SetBGColor(color);
		if (iter->ReadColor("text",&color))
			m_includedStyle->SetColor(color);
		if (iter->ReadColor("link",&color))
			m_includedStyle->SetLinkColor(color);
	}
	view->SetFrameColor(m_includedStyle);
}

void BODY_DocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	if (onlyIfChanged) return;
	TagDocElem::draw(view);
}

void LINK_DocElem::RelationSet(HTMLFrame *view) {
	StrRef link_name;
	StrRef rlink_name;
	StrRef url_attr;
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		iter->ReadStrRef("REL", &link_name);
		iter->ReadStrRef("REV", &rlink_name);
		iter->ReadStrRef("HREF", &url_attr);
	}
	const char *favLinkName = "no name";
	if (!link_name.IsFree()) {
		favLinkName = link_name.Str();
	} else if (!rlink_name.IsFree()) {
		favLinkName = rlink_name.Str();
	}
	if (!strcasecmp(favLinkName,"icon")
	 || !strcasecmp(favLinkName,"shortcut icon")) {
		if (Pref::Default.ShowImages()) {
			Url *url = new Url(url_attr.Str(), view->m_document.CurrentUrl());
			Resource *r = url->GetDataNow();
			BBitmap *bmp = NULL;
			if (r && r->CachedFile()) {
				bmp = BTranslationUtils::GetBitmapFile(r->CachedFile());
			}
			if (r && r->Data()) {
				bmp = BTranslationUtils::GetBitmap(r->Data());
			}
			view->m_document.SetIcon(bmp);
		}
	} else {
		fprintf(stderr, "Link : %s to %s\n", favLinkName, url_attr.Str());
		view->m_document.AddLink(&link_name, &url_attr);
	}

	TagDocElem::RelationSet(view);
}	
