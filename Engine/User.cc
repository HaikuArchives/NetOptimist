#include "User.h"
#include <stdio.h>

void User::Alert(const char *msg) {
	fprintf(stderr, "Alert: %s\n", msg);
}

void User::Message(const char *msg, msg_level_t level) {
	static const char *msg_level_text[] = { "debug", "info", "msg", __FILE__, __FILE__ };
	fprintf(stderr, "%s: %s\n", msg_level_text[level], msg);
}
