#ifndef traces_H
#define traces_H

enum TraceLevel {
	DEBUG_TAGINPAGE = (1<<0),
	DEBUG_TAG = (1<<1),
	DEBUG_COLOR = (1<<2),
	DEBUG_PARSER = (1<<3),
	DEBUG_RENDER = (1<<4),
	DEBUG_MESSAGING = (1<<5),
	DEBUG_URL = (1<<6),
	DEBUG_CACHE = (1<<7),
	DEBUG_CORRECT = (1<<8),
	DEBUG_MSG = (1<<9),
	DEBUG_LEXER = (1<<10),
	DEBUG_EVENT = (1<<11),
	DEBUG_HTTP_DATE = (1<<12),
	DEBUG_HTTP_DATA = (1<<13),
	DEBUG_HTTP = (1<<14),
	DEBUG_HTTP_DUMP_HEADER = (1<<15),
	FILLRECT = (1<<16),
	TAG_STACK = (1<<17),
	DEBUG_TAGATTR = (1<<18),
	DEBUG_TAGPARSE = (1<<19),
	BE_LIB = (1<<20),
	DEBUG_TABLE_RENDER = (1<<21),
	WRITE_FWT_FILE = (1<<22),
	DEBUG_FORCETABLEBORDER = (1<<23),
	X11_EVENT = (1<<24),
	DEBUG_FILE = (1<<25),
	DEBUG_FORMS = (1<<26),
	DEBUG_JAVASCRIPT = (1<<27)
};

extern int enableTrace;

#define trace(l) if(l&enableTrace)
#define ISTRACE(l) (l&enableTrace)

#endif
