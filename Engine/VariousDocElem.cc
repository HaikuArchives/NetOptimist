#include "VariousDocElem.h"
#include "Url.h"
#include <translation/TranslationUtils.h>
#include <Bitmap.h>
#include "Frame.h"
#include "Resource.h"
#include "Pref.h"
#include "Style.h"
#include "StrPlus.h"
#include "UrlQuery.h"
#include "traces.h"

void A_DocElem::geometry(HTMLFrame *view) { 
	TagDocElem::geometry(view);
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		iter->ReadStrRef("HREF",&attr_href);
		iter->ReadStrRef("NAME",&m_attr_name);
		iter->ReadStrRef("ALT",&m_attr_alt);
		iter->ReadStrRef("ONCLICK",&m_attr_onclick);
	}
	if (!strnull(attr_href.Str())) {
		// This tag is a link
		trace(DEBUG_TAG)
			printf("A HREF=%s\n", attr_href.Str());
		m_includedStyle = m_style->clone(id);
		m_includedStyle->SetColor(m_style->LinkColor());
		m_includedStyle->SetLink();
		m_includedStyle->SetUnderline();
	} else {
		// this tag is a <A NAME=...>
		// Do nothing but wait Action() to be called
	}
}

bool A_DocElem::IsActive() { 
	return attr_href.Str()!=NULL;
}

bool A_DocElem::Action(::Action action, UrlQuery *href) { 
	if (action == ANCHOR_SEARCH) {
		return (!m_attr_name.IsFree() && !strcasecmp(href->m_name, m_attr_name.Str()));
	} else {
		if (Pref::Default.UseJavaScript() && (action == MOUSE_CLICK) && !m_attr_onclick.IsFree()) {
			href->SetUrl(&m_attr_onclick);
		} else {
			href->SetUrl(&attr_href);
		}
		href->m_name = m_attr_alt.Str();
		return true;
	}
}

void SCRIPT_DocElem::geometry(HTMLFrame *view) { 
	// TODO : TYPE and SRC are currently ignored
	char attr_type[50];
	StrRef attr_language;
	StrRef attr_src;

	TagDocElem::geometry(view);
	attr_type[0] = '\0';

	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		iter->ReadStrl("TYPE",attr_type,sizeof(attr_type));
		iter->ReadStrRef("LANGUAGE",&attr_language);
		iter->ReadStrRef("SRC",&attr_src);
	}
	if (!attr_src.IsFree()) {
		fprintf(stderr, "Warning : JavaScript code in separate file is not supported (id=%d)\n", id);
	}
}

void NOSCRIPT_DocElem::RelationSet(HTMLFrame *view) {
	if (Pref::Default.UseJavaScript()) {
		hidden = m_included;
		m_included = NULL;
	}
	TagDocElem::RelationSet(view);
}


void HR_DocElem::geometry(HTMLFrame *view) {
	dynamicGeometry(view);
}

void HR_DocElem::dynamicGeometry(HTMLFrame *view) {
	int reqW = -1;
	bool dummy;
	w = constraint->Width();
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		iter->ReadDim("WIDTH",&reqW,&dummy,w);
	}
	if (reqW>=0) w = reqW;
	h = 15;
}

void HR_DocElem::draw(HTMLFrame *view, bool /*onlyIfChanged*/) {
	view->DrawRect(x+10,y+5,w-20,3, m_style);
	TagDocElem::draw(view);
}

void LIST_DocElem::geometry(HTMLFrame *view) {
	m_includedStyle = m_style->clone(id);
	m_includedStyle->IncIndent(10);
}

void LI_DocElem::draw(HTMLFrame *view, bool /*onlyIfChanged*/) {
	view->FillCircle(x+5,y+4,3, m_style);
	TagDocElem::draw(view);
}


const char * FrameDocElem::UserName() const {
	if (!m_alt_attr.IsFree())
		return m_alt_attr.Str();
	else if (!strnull(m_name_attr))
		return m_name_attr;
	else
		return NULL;
}

bool FrameDocElem::IsActive() { 
	return !strnull(m_href_attr.Str());
}

bool FrameDocElem::Action(::Action /*action*/, UrlQuery *href) { 
	href->SetUrl(&m_href_attr);
	href->m_name = UserName();
	return href->Url() != NULL;
}

void FrameDocElem::geometry(HTMLFrame *view) { 
	TagDocElem::geometry(view);
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		iter->ReadStrRef("SRC" ,&m_href_attr);
		iter->ReadStrRef("ALT" ,&m_alt_attr);
		iter->ReadStrl("NAME" ,m_name_attr, sizeof m_name_attr);
	}
	const char *userName = UserName();
	char *str;
	if (userName) {
		str = new char[strlen(userName) + strlen(m_href_attr.Str()) + 30];
		sprintf(str, "[Frame: %s (%s)]", userName, m_href_attr.Str());
	} else {
		str = new char[strlen(m_href_attr.Str()) + 30];
		sprintf(str, "[Frame: %s]", m_href_attr.Str());
	}
	m_text.SetToString(str, false);

	view->StringDim(str, m_style,&w,&h);
	fixedW = w;

	if (!strnull(m_href_attr.Str())) {
		m_includedStyle = m_style->clone(id);
		m_includedStyle->SetColor(m_style->LinkColor());
		m_includedStyle->SetLink();
		m_includedStyle->SetUnderline();
	} else {
		m_includedStyle = NULL;
	}
}

void FrameDocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	if (onlyIfChanged) return;
	const Style *s = m_includedStyle?m_includedStyle:m_style;
	view->DrawString(x,y+h-3,w-3,m_text.Str(),s); // XXX I don't really like the "-3" (see Also in "StrDocElem.cc")
	printPosition("Frame_draw");
}

void IMG_DocElem::LoadImage() {
	if (bmp == NULL && url!=NULL && !badImage) {
		Resource *resource;

		resource = url->GetIfAvail();
		if (resource) {
			const char *mimetype = resource->MimeType();
			if (!resource) printf("Image has no resource (yet)\n");
			if (mimetype) {
				if (!strprefix(mimetype, "image/")) {
					/* Mime types should begin with image/...
					   For example, when image is "404 not found", the mimetype returned
					   is text/html. Of course, Trying to decode a image from an html
					   file may just fail, but actually, with ImageMagick it sometimes
					   crashes the browser. This is why we filter image mimetypes here.
					*/
					fprintf(stderr, "IMG_DocElem::LoadImage : wrong mimetype = %s\n", mimetype);
					badImage = true;
					return;
				}
			}
			if (!resource->Data() && !resource->CachedFile()) {
				fprintf(stderr, "ERROR IMG_DocElem::LoadImage: url %s downloaded but no data\n", m_attr_src.Str());
			} else {
				if (resource->CachedFile()) {
					bmp = BTranslationUtils::GetBitmapFile(resource->CachedFile());
				}
				if (resource->Data()) {
					bmp = BTranslationUtils::GetBitmap(resource->Data());
				}
				if (!bmp) {
					fprintf(stderr, "LoadImage : could not create image from data\n");
					badImage = true;
				}
			}
			if (bmp && !bmp->IsValid()) printf("Image bitmap is invalid\n");
			if (bmp != NULL && !bmp->IsValid()) {
				delete bmp;
				bmp = NULL;
				badImage = true;
				fprintf(stderr, "Url %s cannot be translated into image\n", m_attr_src.Str()?m_attr_src.Str():"no_name" );
			}
		}
	}
	GetSize();
}

void IMG_DocElem::draw(HTMLFrame *view, bool onlyIfChanged) {
	bool needRedraw = !bmp || !onlyIfChanged;
	LoadImage();
	if (bmp) {
		// Draw image
		if (needRedraw) {
			view->FillRect(x+attr_hspace+attr_border,y+attr_vspace+attr_border,imgW,imgH,m_style);
			view->DrawImg(x+attr_hspace+attr_border,y+attr_vspace+attr_border,imgW,imgH,bmp);
		}
	} else {
		// Image not available : draw a gray rect
		view->DrawBadImg(x+attr_hspace+attr_border,y+attr_vspace+attr_border,imgW,imgH,m_style);
		if (!strnull(m_attr_alt.Str())) {
			// Image not available : draw the "alt" text
			int strh, strw;
			view->StringDim(m_attr_alt.Str(), m_style,&strw,&strh);
			if (strw<imgW) {
				view->DrawString(x+attr_hspace,y+attr_vspace+strh,0,m_attr_alt.Str(),m_style);
			}
		}
	}
	if (attr_border>0 && m_style->IsLink()) {
		view->DrawRect(x+attr_hspace,y+attr_vspace,imgW+2*(attr_hspace + attr_border),imgH+2*(attr_hspace + attr_border), m_style);
	}
	TagDocElem::draw(view);
}

void IMG_DocElem::geometry(HTMLFrame *view) {
	imgH=imgW=15;
	w = 0;

	// By default, we draw a border is the image is clickable
	attr_border = m_style->IsLink() ? 1 : 0;

	bool dummy;
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		iter->ReadInt("HEIGHT",&reqH);
		iter->ReadDim("WIDTH",&reqW,&dummy,w);
		iter->ReadInt("HSPACE",&attr_hspace);
		iter->ReadInt("VSPACE",&attr_vspace);
		iter->ReadStrRef("ALT",&m_attr_alt);
		iter->ReadStrRef("SRC",&m_attr_src);
		iter->ReadAlignment("ALIGN",& attr_align);
		iter->ReadInt("border",&attr_border);
	}
	if (m_attr_alt.IsFree() && !m_attr_src.IsFree()) {
		// Not Alt tag : display the filename
		const char *image_src = m_attr_src.Str();
		const char *filename = strrchr(image_src, '/');
		m_attr_alt.SetToDup(filename ? filename+1 : image_src);
	}
	if (!url && !strnull(m_attr_src.Str()) && Pref::Default.ShowImages()) {
			// download in background and reformat only if size is not in html file
		UrlQuery query;
		query.SetUrl(&m_attr_src);
		url = new Url(&query, view->m_document.CurrentUrl(), true, !(reqH>0 && reqW>0 &&!dummy) );
	}
	LoadImage();
}

void IMG_DocElem::GetSize() {
	if (bmp) {
		BRect bounds = bmp->Bounds();
		imgH = (int)bounds.Height();
		imgW = (int)bounds.Width();
	}
	if (reqW>=0) {
		imgW = reqW;
	}
	if (reqH>=0) {
		imgH = reqH;
	}
	w = imgW + 2 * (attr_hspace + attr_border);
	h = imgH + 2 * (attr_vspace + attr_border);
	fixedW = w;
}

void IMG_DocElem::dynamicGeometry(HTMLFrame *view) {
	for (TagAttr *iter = list; iter!=NULL; iter=iter->Next()) {
		bool dummy;
		if (iter->ReadDim("WIDTH",&reqW,&dummy,constraint->Width())) {
			//printf("img parent w=%d, img w=%d\n", constraint->Width(), w);
		}
		// XXX this one won't work now
		//iter->ReadDim("HEIGHT",&reqH,&dummy,h);
	}
	GetSize();
	super::dynamicGeometry(view);
}

IMG_DocElem::~IMG_DocElem() {
	if (url) { delete url; }
	if (bmp) { delete bmp; }
}

