#include "ResourceGetter.h"
#include "Url.h"
#include "UrlQuery.h"
#include "Resource.h"
#ifdef __BEOS__
 #include <OS.h>
#endif
#include <be/app/Message.h>
#include <stdio.h>


ResourceGetter::ResourceGetter(BLooper *notify) : BLooper("Http worker") {
	m_notify = notify;
}

void ResourceGetter::MessageReceived(BMessage *msg) {
	const char *urlStr;
	switch(msg->what) {
		case GET_IMAGE:
			msg->FindString("url", &urlStr);
			fprintf(stderr, "ResourceGetter: got work : url %s\n", urlStr);
			if (urlStr) {
				Resource *rsc;
				UrlQuery query;	 // XXX Should not create new UrlQuery. Reuse existing one instead.
				query.m_method = METHOD_NORMAL;
				query.SetUrl(urlStr);
				query.m_name = NULL;
				query.m_nbFields = 0; // XXX Should have a ctor
				Url url(&query, NULL); // XXX verify that this url is absolute
				rsc = url.GetDataNow(); // This call MUST be synchronous !!!
				if (rsc) {
					if (!rsc->Data() && !rsc->CachedFile()) {
						fprintf(stderr, "ERROR ResourceGetter: url %s downloaded but no data\n", urlStr);
					}
				}
				if (m_notify) {
					bool reformat = true;
					msg->FindBool("reformat", &reformat);
					BMessage msgNotifyEnd(URL_LOADED);
					msgNotifyEnd.AddBool("reformat", reformat);
					m_notify->PostMessage(&msgNotifyEnd);
				} else {
					fprintf(stderr, "ResourceGetter: cannot notify download completed\n");
				}
			}
			break;
		default:
			fprintf(stderr, "Unknown message for ResourceGetter::MessageReceived %s\n", (char*)&msg->what);
	}
}

