#ifndef platform_H
#define platform_H

#ifdef __sun
#include <stdlib.h> // this one must be included first because of redefinition of abs in macros.h
#include <macros.h>
typedef int bool;
typedef int status_t;

const int true = (0==0);
const int false = !true;
#define ASSERT assert
static inline void create_directory(const char *, int) {}
#endif

#ifdef __alpha
#define ASSERT(X)
static inline int max(int a, int b) { return a>b?a:b; }
static inline int min(int a, int b) { return a>b?b:a; }
static inline void create_directory(const char *, int) {}
#endif

#ifdef __BEOS__
static inline int max(int a, int b) { return a>b?a:b; }
static inline int min(int a, int b) { return a>b?b:a; }
#ifndef FILENAME_MAX // Hey why is this not defined ?
#define FILENAME_MAX 256
#endif
#endif

#define app_signature "application/x-vnd.NetOptimist"

#endif
