#ifndef html_H
#define html_H

#include <stdio.h>
#include "Event.h"
#include "Frame.h"
#include <Rect.h>

class DocElem;
class TagDocElem;
class Resource;
class Tag;
class UrlQuery;
class TagStack;
class HTMLWindow;

class BufferReader;
class JsCtx;

class DocFormater {
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
	int m_nb_format;

	TagDocElem* html_parse_tag();
	void html_ctrlchar_alter(char *wholestr, char *& ptr);
	void AddString(const char *string);
	void AddDocElem(DocElem *elem);
	void AddTagDocElem(TagDocElem *elem);
	void CloseDocElem(const Tag *elem);
	void ForceCloseHead();
	void format();
public:
	DocFormater() {
		m_frame = NULL;
		doc = cur = last = NULL;
		m_relationAlreadySet = false;
		m_jsctx = NULL;
		m_nb_format = 1;
	}
	virtual ~DocFormater();
	void parse_html(Resource *resource);
	void parse_text(Resource *resource);
	void AttachToFrame(HTMLFrame *frame);
	void AttachToWindow(HTMLWindow *window);
	void Draw(BRect r, bool onlyIfChanged = false);
	bool Select(int x, int y, Action action, UrlQuery *name);
	void InvalidateLayout() {
		m_nb_format++;
	}
	void InsertText(const char *text, int len);
};

#endif
