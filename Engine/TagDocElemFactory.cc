#include "TagDocElemFactory.h"

#include "Table.h"
#include "MetaDocElem.h"
#include "VariousDocElem.h"
#include "TextDocElem.h"
#include "FormsDocElem.h"

TagDocElem* TagDocElemFactory::New(Tag *t, TagAttr* attrList) {
	if (t->info && !t->closing) {
		const char *tagName = t->toString();
		if (!strcmp(tagName,"TH")) {
			return new TD_DocElem(t, attrList); // XXX This is bad : we handle <TH> like <TD>
		}
		if (!strcmp(tagName,"TD")) {
			return new TD_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"TR")) {
			return new TR_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"TABLE")) {
			return new TableDocElem(t, attrList);
		}
		if (!strcmp(tagName,"B") || !strcmp(tagName,"STRONG")) {
			return new B_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"I") || !strcmp(tagName,"EM")) {
			return new I_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"U")) {
			return new U_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"SMALL")) {
			return new SMALL_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"CODE") || !strcmp(tagName,"VAR")) {
			return new FIXED_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"BIG")) {
			return new BIG_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"H1")
			|| !strcmp(tagName,"H2")
			|| !strcmp(tagName,"H3")
			|| !strcmp(tagName,"H4")
			|| !strcmp(tagName,"H5")
			|| !strcmp(tagName,"H6")
		) {
			return new H1_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"A")) {
			return new A_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"FONT")) {
			return new FONT_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"HR")) {
			return new HR_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"LI")) {
			return new LI_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"UL") || !strcmp(tagName,"OL")) {
			return new LIST_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"IMG")) {
			return new IMG_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"TITLE")) {
			return new TITLE_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"BODY")) {
			return new BODY_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"SCRIPT")) {
			return new SCRIPT_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"NOSCRIPT")) {
			return new NOSCRIPT_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"STYLE")) {
			return new STYLE_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"FORM")) {
			return new FORM_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"INPUT")) {
			return new INPUT_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"OPTION")) {
			return new OPTION_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"BLOCKQUOTE") || !strcmp(tagName,"CITE")) {
			return new BLOCKQUOTE_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"LINK")) {
			return new LINK_DocElem(t, attrList);
		}
		if (!strcmp(tagName,"FRAME")) {
			return new FrameDocElem(t, attrList);
		}
		if (!strcmp(tagName,"META")) {
			return new MetaDocElem(t, attrList);
		}
		
		// These are deprecated tage that won't be supported
		if (!strcmp(tagName,"BASEFONT")) {
			return new TagDocElem(t, attrList);
		}
		
		if (strcmp(tagName,"P") &&
		    strcmp(tagName,"BR") &&
		    strcmp(tagName,"HTML") &&
		    strcmp(tagName,"HEAD") &&
		    strcmp(tagName,"PRE") &&	// This is as parse tag only.
		    strcmp(tagName,"!--")) {

			// THESE ARE UNIMPLEMENTED TAGS !!!!
			// feel free to provide something
			if (
			    strcmp(tagName,"NOBR") &&
			    strcmp(tagName,"Q") &&
			    strcmp(tagName,"DEL") &&
			    strcmp(tagName,"INS") &&
			    strcmp(tagName,"SPAN") &&
			    strcmp(tagName,"CAPTION") &&
			    strcmp(tagName,"THEAD") &&
			    strcmp(tagName,"TFOOT") &&
			    strcmp(tagName,"TBODY") &&
			    strcmp(tagName,"OBJECT") &&
			    strcmp(tagName,"DIV") &&
			    strcmp(tagName,"CENTER") &&
			    strcmp(tagName,"COLGROUP") &&
			    strcmp(tagName,"COL") &&
			    strcmp(tagName,"LEGEND") &&
			    strcmp(tagName,"FIELDSET") &&
			    strcmp(tagName,"FRAMESET") &&
			    strcmp(tagName,"NOFRAMES") &&
			    strcmp(tagName,"IFRAME") &&
			    strcmp(tagName,"LAYER") &&
			    strcmp(tagName,"ILAYER") &&
			    strcmp(tagName,"NOLAYER") &&
			    strcmp(tagName,"MAP") &&
			    strcmp(tagName,"AREA") &&
			    strcmp(tagName,"BASE") && // XXX this one is urgent
			    strcmp(tagName,"!DOCTYPE") && // XXX Is this one usefull for browsers ?
			    strcmp(tagName,"BUTTON") && // Forms
			    strcmp(tagName,"OPTION") && // Forms
			    strcmp(tagName,"SELECT") && // Forms
			    strcmp(tagName,"ADDRESS") &&	// XXX This one is easy
			    strcmp(tagName,"SUP") &&
			    strcmp(tagName,"DIR") &&
			    strcmp(tagName,"TT") &&
			    strcmp(tagName,"DL") &&
			    strcmp(tagName,"DD") &&
			    strcmp(tagName,"DT")) {

				fprintf(stderr, "Warning: TagDocElemFactory : no Tag class for %s\n", tagName);
			}
		}
	}
	
	// The default tag doc elem
	return new TagDocElem(t, attrList);
}
