#include "StatusBarView.h"

void StatusBarView::Draw(BRect updateRect) {
	if (!strnull(msg)) {
		this->DrawString(msg, BPoint(5,13));
	}
}

