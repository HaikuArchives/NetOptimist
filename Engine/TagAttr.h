#ifndef TagAttr_H
#define TagAttr_H

#include "platform.h"
#include "Colors.h"
#include "Alignment.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "StrPlus.h"
#include "traces.h"

class TagAttr {
	static const char *attrNames[];
	TagAttr *next;
	const char *name;
	StrRef m_value;
	void _init(const char*s);
	void Int2Color(unsigned int val, rgb_color *color) const;
#ifdef HASMUTABLE
	// This field is for debug only : it is use to record whether the attribute has been read
	mutable bool read;
#else
	bool read;
#endif
public:
	TagAttr(const char*s, TagAttr *list = NULL);
	~TagAttr() {
		trace(DEBUG_TAGATTR) if (!read && name) {
			fprintf(stderr, "warning, attribute %s=%s not read\n", name?name:"(noname)", m_value.Str()?m_value.Str():"(novalue)");
		}
	}
	TagAttr *Next() const { return next; }
	const char *AttrName() const {
		return name;
	}
	const char *AttrValue() const {
		return m_value.Str();
	}
	bool ReadDim(const char *attrname, int *size, bool *percent, int refSize) const;
	bool ReadSize(const char *s, int *val) const;
	bool ReadInt(const char *s, int *val) const {
		if (name && !strcasecmp(name,s)) {
			if (strnull(m_value.Str())) {
				/* For example, in tag <TABLE BORDER>, we assume that the implicit value is 1
				   I am not sure this is good but fixes tables... */
				*val = 1;
				trace(DEBUG_TAGATTR)
					printf("No explicit value for attribute %s, returning 1\n", name);
			} else {
				*val=atoi(m_value.Str());
			}
			DebugRead();
			return true;
		} else {
			return false;
		}
	}
	bool ReadStrRef(const char *attrname, StrRef *value_out) const {
		if ((name==NULL && attrname==NULL) || (name && attrname && !strcasecmp(name,attrname))) {
			value_out->SetToRef(&m_value);
			DebugRead();
			return true;
		} else {
			return false;
		}
	}
	bool ReadStrp(const char *attrname, const char **value_out) const {
		if ((name==NULL && attrname==NULL) || (name && attrname && !strcasecmp(name,attrname))) {
			*value_out = m_value.Str();
			DebugRead();
			return true;
		} else {
			return false;
		}
	}

	bool ReadStrl(const char *attrname, char *value_out, int value_len) const {
		/* This function assumes `value_len` bytes have been allocated for `value_out`,
		   the last byte is '\0' */
		if (name && !strcasecmp(name,attrname)) {
			value_out[value_len-1]='\0';
			strncpy(value_out, m_value.Str(), value_len-1);
			DebugRead();
			return true;
		} else {
			return false;
		}
	}
	bool ReadColor(const char *attrname, rgb_color *color) const;
	bool ReadAlignment(const char *attrname, Alignment *aligntype) const;
	void DebugRead() const {
		// not all compilers have "mutable"
		 (const_cast<TagAttr*>(this))->read = true;
	}
};

#endif
