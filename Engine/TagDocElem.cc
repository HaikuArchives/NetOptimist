#include <Rect.h>
#ifdef __BEOS__
# include <be/support/Debug.h>
#endif

#include "platform.h"
#include "TagDocElem.h"
#include "Constraint.h"
#include "Style.h"


TagDocElem::~TagDocElem() {
	TagAttr *attr = list;
	while(attr != NULL) {
		TagAttr *next = attr->Next();
		delete attr;
		attr = next;
	}
	delete t;
}
void TagDocElem::place() {
	if (t->info) {
		if (!t->closing) {
			if (t->info->css_display == D_BLOCK && t->info->marginTop >= 0) {
				constraint->NewLine(m_style->Indent(), t->info->marginTop);
			}
		}
	}

	constraint->NextPosition(&x, &y, w, h, m_style->Indent(), m_style->Align());

	if (t->info) {
		if (t->closing || t->info->closeMode==NEVER_CLOSE) {
			if (t->info->css_display == D_BLOCK && t->info->marginBottom >= 0) {
				constraint->NewLine(m_style->Indent(), t->info->marginBottom);
			}
		}
	}

	if (x+w-1 > constraint->MaxX() && constraint->creator) {
		fprintf(stderr, "Warning: Tag %s(%d): constraint from %s(%d) too small : got %d, needs %d\n", toString(), id,  constraint->creator->toString(), constraint->creator->id, constraint->MaxX(), x+w-1);
		constraint->creator->update(x+w-1, y, y+h-1);
	}
	printPosition("Tag_place");
}

void TagDocElem::draw(HTMLFrame *, const BRect *, bool onlyIfChanged) {
	printPosition("Tag_draw");
}

bool TagDocElem::close(const Tag *symbol, DocElem *last) {
	if (symbol->closes(t) && m_included==NULL) {
		m_included = m_next;
		m_included->Inclosed(this);
		m_next = NULL;
		last->ClosingParent(this);
		return true;
	} else {
		return false;
	}
}
