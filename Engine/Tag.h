#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "platform.h"

typedef enum { ALWAYS_CLOSE, NEVER_CLOSE, CAN_CLOSE } CloseMode;
typedef enum { D_INLINE, D_BLOCK, D_NONE } Css_display;

struct TagInfo {
	const char *name;
	Css_display css_display;
	int marginTop;
	int marginBottom;
	CloseMode closeMode;
	bool canReopen; // if false tag is closed before reopening it
	static const TagInfo tagInfos[];
	// These are static because we want to use {}-initializer
	static const char *Name(const TagInfo *t) { return t->name + 1; }
	static const char *ClosingName(const TagInfo *t) { return t->name; }
};

class Tag {
	char*name;
public:
	bool closing;
	const TagInfo *info;
	Tag() { closing = false; info = NULL; name = NULL; }
	Tag(const char *s, bool isClosing = false) {
		info = NULL;
		name = NULL;
		closing = isClosing || (s[0] == '/');
		if (s[0] == '/') {
			s++;
		}
		for(const TagInfo *tagInfo=TagInfo::tagInfos; tagInfo->name != NULL; tagInfo++) {
			if (!strcasecmp(TagInfo::Name(tagInfo),s)) {
				info = tagInfo;
				break;
			}
		}
		if (info == NULL) {
			fprintf(stderr,"Tag %s is unknown\n", s);
			name = strdup(s); for (char*c=name; *c!='\0'; c++) *c=toupper(*c);
		}
		if (closing && info && info->closeMode==NEVER_CLOSE) {
			fprintf(stderr,"ERROR : tag %s was not supposed to be closable\n", TagInfo::Name(info));
		}
	}
	~Tag() { if (name) free(name); }
	const char *toString() const {
		if (info) {
			return closing ? TagInfo::ClosingName(info) : TagInfo::Name(info);
		} else {
			return name ? name : "UnknowClosingTag";
		}
	}
	int closes(const Tag *t) const {
		const char *n1, *n2;
		if (t->closing) {
			return false;
		}
		n1 = info ? TagInfo::Name(info) : name;
		n2 = t->toString();
		return (closing && ( (n1 == n2) || (strcmp(n1,n2)==0) ));
	}
};
