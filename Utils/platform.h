#ifndef platform_H
#define platform_H

#ifdef __sun
#include <stdlib.h> // this one must be included first because of redefinition of abs in macros.h
#include <macros.h>
typedef int bool;
typedef int status_t;

const int true = (0==0);
const int false = !true;
#define B_FILE_NAME_LENGTH FILENAME_MAX
#define ASSERT assert
static inline void create_directory(const char *, int) {}
#endif

#ifdef __alpha
#define ASSERT(X)
static inline int max(int a, int b) { return a>b?a:b; }
static inline int min(int a, int b) { return a>b?b:a; }
static inline void create_directory(const char *, int) {}
#endif

#if defined(__BEOS__) || defined(__HAIKU__)
#include <Debug.h>
static inline int max(int a, int b) { return a>b?a:b; }
static inline int min(int a, int b) { return a>b?b:a; }
#else
#include "Be.h"
#endif

#define app_signature "application/x-vnd.NetOptimist"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	256
#endif

// Useful macros
#define FREE(p) if (p) { free(p); p=NULL; }
#define DELETE(p) if (p) { delete(p); p=NULL; }
#define STRDUP(p) ((p)?strdup(p):NULL)

#endif
