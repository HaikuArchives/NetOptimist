#ifndef HISTORY_H
#define HISTORY_H

#include <time.h>
#include "StrPlus.h"

class History {
        struct HistLine {
                HistLine *prev;
                HistLine *next;
                StrRef url;
                StrRef title;
                time_t lastVisited;
                int nbVisits;
        };
        HistLine *last;
        HistLine *cur;
        History();
        ~History();
public:
        static History history;
        StrRef *Back();
        StrRef *Forward();
        void Add(const StrRef *);
        void Query(const char *pattern);
};

#endif
