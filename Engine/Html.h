#ifndef html_H
#define html_H

#include <stdio.h>
#include "Event.h"
#include "Frame.h"

enum DocFormaterMsg {
	FORMAT = 'FORM',
	EXPOSE = 'EXPO',
	EXPOSE_IF_CHANGED = 'EXND'
};

class DocElem;
class TagDocElem;
class Resource;
class Tag;
class UrlQuery;
class TagStack;

class BufferReader;
class JsCtx;

class DocFormater : public EventProcessor {
private:
	HTMLFrame *m_frame;		// ptr to graphical object
	BufferReader *m_bufferReader;	// helper object to read html streams
	DocElem *doc;			// Document (root document element)
	bool m_relationAlreadySet;
	JsCtx *m_jsctx;			// JS VM for this document

	// Those fields are used during parsing only :
	DocElem *cur;			// current elem
	DocElem *last;			// current last elem
	TagStack *m_openedTags;		// stack of opened html tags
	int	m_parseMode;

	TagDocElem* html_parse_tag();
	void html_ctrlchar_alter(char *wholestr, char *& ptr);
	void AddDocElem(DocElem *elem);
	void AddTagDocElem(TagDocElem *elem);
	void CloseDocElem(const Tag *elem);
	void ForceCloseHead();
	void format();
	void Draw(bool onlyIfChanged = false);
public:
	DocFormater(HTMLFrame *view) {
		m_frame = view;
		doc = cur = last = NULL;
		m_relationAlreadySet = false;
		m_jsctx = NULL;
	}
	virtual ~DocFormater();
	void parse_html(Resource *resource);
	bool Select(int x, int y, Action action, UrlQuery *name);
	void Process(int evt) {
		switch(evt) {
		case EXPOSE:
			Draw();
			break;
		case EXPOSE_IF_CHANGED:
			Draw(true);
			break;
		case FORMAT:
			format();
			break;
		default:
			fprintf(stderr,"DocFormater: Message not handled\n");
			break;
		}
	}
	void InsertText(const char *text, int len);
};

#endif
