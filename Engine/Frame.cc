
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
	m_nextUrl = NULL;
	m_getter = new ResourceGetter(m_container);
	m_getter->Run();
}

HTMLFrame::~HTMLFrame() {
	m_getter->Lock();
	m_getter->Quit();
	//if (m_getter) delete m_getter;	// XXX Should we delete that ?
	if (m_document.CurrentUrl()) delete m_document.CurrentUrl();
	if (m_format) delete m_format;
}

void HTMLFrame::ModifyUrl(Url *newUrl) {
	Cache::cache.SetResourceGetter(m_getter);

	Resource *rsc = newUrl->GetIfAvail();
	if (rsc) {
		if (m_document.CurrentUrl()) delete m_document.CurrentUrl();
		if (m_format) delete m_format;

		m_document.SetUrl(newUrl);

		m_format = new DocFormater();
		ResourceProcessor::Process(newUrl);

		printf("HTMLFrame::ModifyUrl() : parse_html\n");
		m_format->parse_html(rsc);
		m_format->AttachToFrame(this);
		printf("HTMLFrame::ModifyUrl() : Refresh\n");
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
	} else {
		// XXX we should notify the user that this
		// XXX resource is not available (anymore ?)
	}
}

void HTMLFrame::SetToUrl(Url *newUrl) {
	ModifyUrl(newUrl);
	StrRef ref;
	newUrl->ToStrRef(&ref);
	History::history.Add(&ref);
}

bool HTMLFrame::Select(int viewX, int viewY, Action action) {
	UrlQuery *query = new UrlQuery;
	bool success = false;
	if (!m_format) return false;
	success = m_format->Select(viewX, viewY, action, query);

	switch (action) {
	case MOUSE_CLICK :
		if (success && !strnull(query->Url())) {
			char buf[80];
			snprintf(buf, sizeof(buf)-1, "New URL selected : %s (name=%s)", query->Url(), query->m_name?query->m_name:"none");
			Message(buf, LVL_INFO);
			SetUrl(query);	// Pass ownership of query (ie do not delete it)
			return true;
		}
		break;
	case MOUSE_OVER :
	case MOUSE_ALTCLICK :
	case ANCHOR_SEARCH :
		break;
	}
	delete query;
	return success;
}
void HTMLFrame::Back() {
	if (!History::history.HasBack())
		return;

	UrlQuery query;
	query.SetUrl(History::history.Back());
	Url *newUrl = new Url(&query, NULL);
	ModifyUrl(newUrl);
}

void HTMLFrame::Forward() {
	if (!History::history.HasForward())
		return;

	UrlQuery query;
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
	UrlQuery *query = new UrlQuery;

	query->SetUrl(url_text);
	SetUrl(query);
}

void HTMLFrame::SetUrl(UrlQuery *urlQuery) {
	Cache::cache.SetResourceGetter(m_getter);
	m_nextQuery = urlQuery;
	m_nextUrl = new Url(m_nextQuery, m_document.CurrentUrl(), true, true);
	IncResourceWaiting();
	CheckUrl();
}

void HTMLFrame::CheckUrl() {
	Resource *rsc;

	if (m_nextUrl==NULL) {
		printf("HTMLFrame::CheckUrl() not waiting\n");
		return; // Nothing to do.
	}

	rsc = m_nextUrl->GetIfAvail();

	while ((rsc && rsc->Size()==0) && m_nextQuery->m_maxHop>0 && m_nextUrl) {
		Url *previousUrl;
		previousUrl = m_nextUrl;
		m_nextQuery->m_maxHop--;

		if (rsc->m_location && strcmp(rsc->m_location, m_nextQuery->Url())) {
			// Try this new location
			printf("HTMLFrame::CheckUrl() trying new location : %s\n", rsc->m_location);
			m_nextQuery->SetUrl(rsc->m_location);
		} else {
			printf("HTMLFrame::CheckUrl() url empty but and no new location \n");
		}
		m_nextUrl = new Url(m_nextQuery, previousUrl, true, true);
		rsc = m_nextUrl->GetIfAvail();

		delete previousUrl;
	}

	if (rsc && rsc->Size()>0) {
		// Success for this url
		printf("HTMLFrame::CheckUrl() success\n");
		SetToUrl(m_nextUrl);
		m_nextUrl = NULL;
		DecResourceWaiting();
		delete m_nextQuery;
	} else {
		printf("HTMLFrame::CheckUrl() waiting but not avail\n");
	}
}

void HTMLFrame::Resize(int w, int h) {
	m_winw = w;
	m_winh = h;
}
