/*
	fwt applies a subset of the syntax of TeX to HTML document :
	- more compact
	- easier to read
	- easier to parse
	but
	- not standard at all
	- problem with comments and JS code

	Examples :

	1) a simple table
	\table{
		\tr{
			\td{A cell}
		}
	}
	
	2) a <BR>
	\br{}

	3) Comments : like C comments

	4) image
	\img[src="img/logo.gif", alt="Logo"]{}

	Notes :
	- there is no closing tags.
	- {} are mandatory but [] are optional
*/

#include "DocElem.h"
#include "StrDocElem.h"
#include "TagDocElem.h"
#include "DocWalker.h"

int ExportToFWT(DocElem *doc) {
	FILE *file;
	file = fopen("tst.fwt", "w");
	DocWalker walk(doc);
	DocElem *iter;
	int tagOccured=0;
	while ((iter = walk.Next())) {
		walk.Feed(iter);
		walk.FeedHidden(iter);

		TagDocElem *tag;
		tag = dynamic_cast<TagDocElem *>(iter);
		if (tag && tag->isClosing()) {
			if (tagOccured)
				fprintf(file, "\n}");
			else
				fprintf(file, "}");
			tagOccured=1;
		} else if (tag) {
			fprintf(file, "\n\\%s.%d", tag->toString(), tag->id);
			TagAttr *attr = NULL;
			if (tag)
				attr = tag->AttributeList();
			if (attr) {
				fprintf(file, "[");
				while(attr!=NULL) {
					if (attr->AttrName() && attr->AttrValue())
						fprintf(file, "%s=\"%s\"",attr->AttrName(),attr->AttrValue());
					else
						fprintf(file, "%s",attr->AttrValue());
					attr = attr->Next();
					if (attr)
						fprintf(file, ", ");
					else
						fprintf(file, "]"); // This was the last tag attribute, close the list
				}
			}
			if (!iter->Included() && !iter->Hides()) {
				fprintf(file, "-");
			} else {
				fprintf(file, "{");
				tagOccured=0;
			}
		} else {
			StrDocElem *str;
			str = dynamic_cast<StrDocElem *>(iter);
			fprintf(file, "%s ", str->printSpecific());
		}
	}
	fclose(file);
	return 0;
}

