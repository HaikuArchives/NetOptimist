#ifndef FormsDocElem_H
#define FormsDocElem_H

#include "TagDocElem.h"
#include "UrlQuery.h"

class INPUT_DocElem;

class FORM_DocElem : public TagDocElem {
	ProtocolMethod m_method; // GET, POST (PUT too ?)
	const char *m_actionUrl;
	const char *m_name;
	INPUT_DocElem *m_fields[50];
	int m_nbFields;
public:
	FORM_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
		m_method = METHOD_GET;
		m_actionUrl = NULL;
		m_name = NULL;
		m_nbFields = 0;
	}
	void Register(INPUT_DocElem *field);
	virtual void RelationSet(HTMLFrame *view);
	virtual bool Action(::Action action, UrlQuery * query);
};

class INPUT_DocElem : public TagDocElem {
	enum {
		T_SUBMIT,
		T_RESET,
		T_RADIO,
		T_CHECKBOX,
		T_TEXT,
		T_PASSWORD,
		T_IMAGE,
		T_HIDDEN,
		T_BOGGUS	// Unknown input type
	} m_type;
	char m_name[50];
	char m_value[500];
	bool m_activated;
	const char* Text() const;
	FORM_DocElem *m_container;
public:
	INPUT_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
		m_type = T_BOGGUS;
		m_name[0] = '\0';
		m_value[0] = '\0';
		m_container = NULL;
		m_activated = false;
	}
	const char *Name() { return m_name; }
	virtual void geometry(HTMLFrame *view);
	virtual void draw(HTMLFrame *m_view, bool onlyIfChanged=false);
	virtual void RelationSet(HTMLFrame *view);
	virtual bool IsActive();
	virtual bool Action(::Action action, UrlQuery * href);
	void PrintState(char *str);
	bool Activated() const;
};

class OPTION_DocElem : public TagDocElem {
	char m_text[50];
	char m_value[50];
	bool m_selected;
	DocElem* m_hidden;
public:
	OPTION_DocElem(Tag *tag, TagAttr *attrs) : TagDocElem(tag,attrs) {
		m_text[0] = '\0';
		m_value[0] = '\0';
		m_selected = false;
	}
	virtual void geometry(HTMLFrame *view);
	//virtual void draw(HTMLFrame *m_view, bool onlyIfChanged=false);
	virtual void RelationSet(HTMLFrame *view);
	virtual DocElem *Hides() const {
		return m_hidden;
	}
};

#endif
