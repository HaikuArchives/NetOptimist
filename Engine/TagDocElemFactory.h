#ifndef TagDocElemFactory_H
#define TagDocElemFactory_H

#include "TagDocElem.h"

class TagDocElemFactory {
public:
	static TagDocElem* New(Tag *t, TagAttr* attrList = NULL);
};

#endif
