#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([NetOptimist],[0.4],[stephane.fritsch@gmail.com])
AC_CONFIG_SRCDIR([BeOS_UI/Colors.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_MAKE_SET

LT_INIT
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lbe':
AC_CHECK_LIB([be], [main])
AC_CHECK_LIB([network], [socket])
# FIXME: Replace `main' with a function in `-ltextencoding':
AC_CHECK_LIB([textencoding], [main])
# FIXME: Replace `main' with a function in `-ltracker':
AC_CHECK_LIB([tracker], [main])
# FIXME: Replace `main' with a function in `-ltranslation':
AC_CHECK_LIB([translation], [main])

# Checks for header files.
AC_CHECK_HEADERS([OS.h arpa/inet.h fcntl.h locale.h malloc.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/param.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

AC_CONFIG_MACRO_DIR([m4])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([gethostbyname inet_ntoa memset socket strcasecmp strchr strcspn strdup strerror strncasecmp strrchr strspn strstr strtol strtoul])

AC_CONFIG_FILES([
		Makefile
                 Engine/Makefile
                 Network/Makefile
                 Utils/Makefile
                 jsbridge/Makefile
		 BeOS_UI/Makefile
])
AC_OUTPUT
