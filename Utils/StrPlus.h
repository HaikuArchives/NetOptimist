#ifndef STRPLUS_H
#define STRPLUS_H

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "platform.h"

/* strnull - return true if str is NULL or an empty c-string */
static inline int strnull(const char *str)
{
	return str==NULL || str[0]=='\0';
}

/* strprefix - if ptr point to a c-string beginning with prefix,
 * the function returns strlen(prefix) else returns 0.
 * This function is useful to parse a string containing keywords.
 */
static inline int strprefix(const char *str, const char *prefix)
{
	int len = 0;
	while(*prefix) {
		if (*prefix==*str) {
			prefix++;
			str++;
			len++;
		} else {
			break;
		}
	}
	return (*prefix)?0:len;
}

static inline int strcaseprefix(const char *str, const char *prefix)
{
	int len = 0;
	while(*prefix) {
		char p = tolower(*prefix);
		char s = tolower(*str);
		if (p==s) {
			prefix++;
			str++;
			len++;
		} else {
			break;
		}
	}
	return (*prefix)?0:len;
}

// Nothing really new in these ones execpt that they use new instead of malloc

static inline char *StrDup(const char *str) {
	char *s = new char[strlen(str)+1];
	strcpy(s, str);
	return s;
}

static inline void StrFree(char *str) {
	delete[] str;
}

#if 0
void StrRelease(StrRef *ref) {
	ref->m_nbrefs--;
	if (ref->m_nbrefs==0 {
		if (!ro) {
			delete[] ref->str;
			ref->str = NULL;
		}
		delete ref;
	}
}
#endif

class StrRef {
public:
	StrRef() {
		m_nbrefs = NULL;
		str = NULL;
	}
	StrRef(const char *s) {
		SetToConst(s);
	}
	void SetToString(char *s, bool static_storage) {
		Free();
		if (static_storage) {
			m_nbrefs = NULL;
			str = s;
		} else {
			m_nbrefs = new int;
			*m_nbrefs = 1;
			str = s;
		}
	}
	void SetToConst(const char *s) {
		Free();
		m_nbrefs = NULL;	// This string is read only
		str = const_cast<char*>(s);
	}
	void SetToDup(const char *s) {
		Free();
		m_nbrefs = new int;
		*m_nbrefs = 1;
		str = StrDup(s);
		//printf("StrDup %s\n", str);
	}
	void SetToRef(const StrRef *ref) {
		Free();
		str = ref->str;
		m_nbrefs = ref->m_nbrefs;
		if (m_nbrefs) {
			//printf("multiple refs(%d) on %s\n", *m_nbrefs, str);
			(*m_nbrefs)++;
		}
	}
	bool IsFree() const { return str==NULL; }
	void Free() {
		if (m_nbrefs) {
			(*m_nbrefs)--;
			if (! *m_nbrefs) {
				delete m_nbrefs;
				//printf("Freeing %s\n", str);
				StrFree(str);
			}
		}
		m_nbrefs = NULL;
		str = NULL;
	}
	~StrRef() {
		Free();
	}
	const char *Str() const { return str; }
	char *Buffer() const { return str; }
private:
	char *str;
		/* m_nbrefs is either NULL indicating that the
		   string has a static storage or points to a
		   int (that must be free when equal to 0)
		   counting the references to the string */
	int *m_nbrefs;
};

#endif
