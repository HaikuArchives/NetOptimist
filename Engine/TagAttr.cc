#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "platform.h"
#include "TagAttr.h"
#include "traces.h"

const char *TagAttr::attrNames[] = {
/* -- GEOMETRY -- */
	"WIDTH",
	"HEIGHT",
	"VALIGN",
	"ALIGN",
	"SIZE",
	"HSPACE",
	"VSPACE",
	"MAXLENGTH",
	"MAXSIZE",
	"LENGTH",
	"MARGINHEIGHT",
	"MARGINWIDTH",
	"TOPMARGIN",
	"LEFTMARGIN",
	"RIGHTMARGIN",
	"CLEAR",	/* for tag BR */
/* -- TABLES -- */
	"COLSPAN",
	"ROWSPAN",
	"CELLPADDING",
	"CELLSPACING",
	"SCROLLING",
	"BORDER",
	"BORDERCOLOR",
	"FRAMEBORDER",
	"FRAMESPACING",
/* -- LINK -- */
	"ALT",
	"HREF",
	"SRC",
	"REL",
	"REV",
	"TARGET",
	"NAME",
	"VALUE",
	"ID",
/* -- COLOR -- */
	"COLOR",
	"BGCOLOR",
	"TEXT",
	"LINK",
	"VLINK",
	"ALINK",
/* -- STYLE -- */
	"STYLE",
/* -- FONT -- */
	"FACE",
/* -- SCRIPT -- */
	"LANGUAGE",
	"CLASS",
	"ONLOAD",
	"ONUNLOAD",
	"ONMOUSEOVER",
	"ONMOUSEOUT",
	"ONCLICK",
	"ONCHANGE",
	"ONSUBMIT",
	"ONFOCUS",
/* -- HEADER -- */
	"CONTENT",
	"CONTENT-TYPE",
	"HTTP-EQUIV",
	"BACKGROUND",
/* -- FORMS -- */
	"TYPE",
	"METHOD",
	"ACTION",
	"TABINDEX",
/* -- MAPS -- */
	"USEMAP",
	"SHAPE",
	"COORDS",
	"TITLE",
	NULL
};

const struct ColorName {
	const char *name;
	const rgb_color *value;
} colorTable[] = {
	{"black",	&kBlack},
	{"white",	&kWhite},
	{"green",	&kGreen},
	{"yellow",	&kYellow},
	{"red",		&kRed},
	{"gray",	&kGray},
	{"blue",	&kBlue},
	{"magenta",	&kMagenta},
	{NULL, NULL}
};

TagAttr::TagAttr(const char*s, TagAttr *list) {
	name = NULL;
	next = list;
	read = false;

	int i;
	for (i=0; s[i]!='\0'; i++) {
		if (s[i]=='=') {
			break;
		}
	}
	if (s[i]=='=' && s[i+1]!='\0') {
		int valueStart;
		if (s[i+1]=='"' || s[i+1]=='\'') valueStart = i+2; else valueStart = i+1;
		m_value.SetToDup(s+valueStart);
		char *str = m_value.Buffer();
		int valueLen = strlen(m_value.Str());
		if (str[valueLen-1]=='"' || str[valueLen-1]=='\'') 
			str[valueLen-1]='\0';
		const char **iter = attrNames;
		while(*iter != NULL) {
			if (strncasecmp(*iter,s,i)==0) {
				name = *iter;
				break;
			}
			iter++;
		}
		if (name == NULL) {
			trace(DEBUG_TAGATTR) fprintf(stderr,"Warning: attribute [%s] unknown\n",s);
			char *t = new char[i+1];
			strncpy(t, s, i);
			t[i]='\0';
			name=t;
		}
	} else {
		m_value.SetToDup(s);
	}
}

void TagAttr::Int2Color(unsigned int val, rgb_color *color) const {
	color->red = val >> 16;
	color->green = (val&0xff00) >> 8;
	color->blue = (val&0xff);
	color->alpha = 0;
}

bool TagAttr::ReadDim(const char *attrname, int *size, bool *percent, int refSize) const {
	if (name && !strcasecmp(name,attrname)) {
		int val;
		val=atoi(m_value.Str());
		int valueLen = strlen(m_value.Str());
		if (valueLen>0 && (m_value.Str())[valueLen-1]=='%') {
			*percent = true;
			*size = val*refSize/100;
		} else {
			*percent = false;
			*size = val;
		}
		DebugRead();
		return true;
	} else {
		return false;
	}
}

bool TagAttr::ReadColor(const char *attrname, rgb_color *color) const {
	if (name && !strcasecmp(name,attrname)) {
		const char *val = m_value.Str();
		const ColorName *knownColor;

#ifndef STRICT_PARSER
		// skip leading ' . I don't think this is allowed in HTML but some pages uses it
		if (val[0]=='\'') {
			val ++;
		}
#endif
		// skip leading #
		if (val[0]=='#') {
			val ++;
		} else {

			// if empty string...
			if (val[0]=='\0') return false;

			for (knownColor = colorTable; knownColor->name != NULL; knownColor++) {
				if (!strncasecmp(val, knownColor->name, strlen(knownColor->name))) {
					*color=*knownColor->value;
					DebugRead();
					return true;
				}
			}
		}
		if (!isxdigit(val[0])) {
			fprintf(stderr,"ERROR : TagAttr::ReadColor not a hexa color : %s %c[=%d]\n",m_value.Str(), val[0], (int)val[0]);
			return false;
		}
		
		Int2Color(strtoul(val,NULL,16),color);
		DebugRead();
		return true;
	} else {
		return false;
	}
}

bool TagAttr::ReadAlignment(const char *attrname, Alignment *aligntype) const {
	if (name && !strcasecmp(name,attrname)) {
		if (!strcasecmp(m_value.Str(), "TOP")) {
			*aligntype = al_top;
		} else if (!strcasecmp(m_value.Str(), "CENTER")) {
			*aligntype = al_center;
		} else if (!strcasecmp(m_value.Str(), "RIGHT")) {
			*aligntype = al_right;
		} else if (!strcasecmp(m_value.Str(), "LEFT")) {
			*aligntype = al_left;
		} else {
			return false;
		}
		DebugRead();
		return true;
	} else {
		return false;
	}
}

bool TagAttr::ReadSize(const char *attrname, int *size) const {
	/* This function should be used to parse 
		SIZE=10
		SIZE=+2
		SIZE=-1
	*/

	/* XXX This function is incomplete :
	   XXX it needs an input size to parse relative size
	*/
	if (name && !strcasecmp(name,attrname)) {
		*size=atoi(m_value.Str());
		DebugRead();
		return true;
	} else {
		return false;
	}
}
