
#include "Url.h"
#include "UrlQuery.h"
#include "Html.h"
#include "EventCodes.h"
#include "ResourceGetter.h"
#include "History.h"
#include "Frame.h"
#include "StrPlus.h"
#include "Resource.h"
#include <Message.h>

#ifndef NB_ELEM
#define NB_ELEM(a) (sizeof(a)/sizeof(a[0]))
#endif

static const char *known_mime_types[] = {
	"text/html",
	"text/plain",
	"image/gif",
	"image/jpg" };

class ResourceProcessor {
public:
	static void Process(Url *url) {
		Resource *rsc = url->GetIfAvail();
		if (!rsc) return;

		const char *mimetype = rsc->MimeType();
		if (!mimetype) return;

		unsigned i;
		for (i = 0; i<NB_ELEM(known_mime_types); i++) {
			if (strcmp(known_mime_types[i], mimetype)==0) {
				printf("%s is a known mimetype\n", mimetype);
				break;
			}
		}
		if (i==NB_ELEM(known_mime_types)) {
			fprintf(stderr, "mimetype = %s is unknown\n", mimetype);
		}
	}

};

HTMLFrame::HTMLFrame(BLooper *container, int w, int h) : m_container(container) {
	Resize(w,h);
	m_format = NULL;
	m_getter = NULL;
	m_getter = new ResourceGetter(m_container);
	m_getter->Run();
}

HTMLFrame::~HTMLFrame() {
	m_getter->Lock();
	m_getter->Quit();
	if (m_getter) delete m_getter;	// XXX Should we delete that ?
	if (m_document.CurrentUrl()) delete m_document.CurrentUrl();
	if (m_format) delete m_format;
}

void HTMLFrame::ModifyUrl(Url *newUrl) {
	Cache::cache.SetResourceGetter(m_getter);

	if (m_document.CurrentUrl()) delete m_document.CurrentUrl();
	if (m_format) delete m_format;

	m_document.SetUrl(newUrl);

	m_format = new DocFormater(this);
	ResourceProcessor::Process(newUrl);
	Resource *rsc = newUrl->GetDataNow();

	m_format->parse_html(rsc);

	Refresh();

#ifdef __BEOS__
	// Notify container
	StrRef ref;
	newUrl->ToStrRef(&ref);

	BMessage msg(URL_MODIFIED);
	msg.AddString("URL", ref.Str());
	m_container->PostMessage(&msg);
#endif
	NewDocumentLoaded();
}

void HTMLFrame::SetToUrl(Url *newUrl) {
	ModifyUrl(newUrl);
	StrRef ref;
	newUrl->ToStrRef(&ref);
	History::history.Add(&ref);
}

bool HTMLFrame::Select(int viewX, int viewY, Action action) {
	UrlQuery query;
	bool success = false;
	if (!m_format) return false;
	success = m_format->Select(viewX, viewY, action, &query);

	switch (action) {
	case MOUSE_CLICK :
		if (success && !strnull(query.Url())) {
			char buf[80];
			snprintf(buf, sizeof(buf)-1, "New URL selected : %s (name=%s)", query.Url(), query.m_name?query.m_name:"none");
			Message(buf, LVL_INFO);
			SetUrl(&query);
			return true;
		}
		break;
	case MOUSE_OVER :
	case MOUSE_ALTCLICK :
	case ANCHOR_SEARCH :
		break;
	}
	return success;
}
void HTMLFrame::Back() {
	UrlQuery query;

	// STAS:
	if (!History::history.Back())
		return;
	// /STAS

	query.SetUrl(History::history.Back());
	Url *newUrl = new Url(&query, NULL);
	ModifyUrl(newUrl);
}

void HTMLFrame::Forward() {
	UrlQuery query;

	// STAS:
	if (!History::history.Forward())
		return;
	// /STAS

	query.SetUrl(History::history.Forward());
	Url *newUrl = new Url(&query, NULL);
	ModifyUrl(newUrl);
}

void HTMLFrame::SetUrl(const char *url_text) {
	// Verify that url is correct
	if (strnull(url_text)) {
		Alert("Invalid URL (empty string)");
		return;
	}
	UrlQuery query;

	query.SetUrl(url_text);
	SetUrl(&query);
}

void HTMLFrame::SetUrl(UrlQuery *urlQuery) {
	Url *newUrl;
	Url *previousUrl;
	Resource *rsc;

	newUrl = new Url(urlQuery, m_document.CurrentUrl());
	rsc = newUrl->GetDataNow();

	while ((rsc && rsc->Size()==0) && urlQuery->m_maxHop>0 && newUrl) {
		urlQuery->m_maxHop--;
		previousUrl = newUrl;

		if (rsc->m_location && strcmp(rsc->m_location, urlQuery->Url())) {
			// Try this new location
			printf("HTMLFrame::SetUrl trying new location : %s\n", rsc->m_location);
			urlQuery->SetUrl(rsc->m_location);
		} else {
			printf("HTMLFrame::SetUrl url empty but and no new location \n");
		}
		newUrl = new Url(urlQuery, previousUrl);
		rsc = newUrl->GetDataNow();

		delete previousUrl;
	}

	if (rsc && rsc->Size()>0) {
		SetToUrl(newUrl);
	} else {
		printf("HTMLFrame::SetUrl no data\n");
	}
}

void HTMLFrame::Resize(int w, int h) {
	m_winw = w;
	m_winh = h;
}
