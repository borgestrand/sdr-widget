#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([ghpsdr3], [1.0], [andrew@montefusco.com])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# compiling `client.c' with per-target flags requires `AM_PROG_CC_C_O' in `configure.ac'
AM_PROG_CC_C_O

# library (in DttSP) used but `RANLIB' is undefined:  the usual way to define `RANLIB' is to add `AC_PROG_RANLIB'
AC_PROG_RANLIB

# Checks for libraries.

# FIXME: Replace `main' with a function in `-lpthread':
have_pthreads=no
AC_SEARCH_LIBS([pthread_create], [pthread],[have_pthreads=yes])

have_portaudio2=no
AC_SEARCH_LIBS([Pa_Initialize], [portaudio],[have_portaudio2=yes])


# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h sys/timeb.h unistd.h pulse/simple.h])

if test "x${have_pthreads}" = xyes; then
    AC_CHECK_HEADERS([pthread.h], [], [have_pthreads=no])
fi

if test "x${have_pthreads}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The pthread library and header file required to build ghpsdr3.
-----------------------------------])

fi


if test "x${have_portaudio2}" = xyes; then
    AC_CHECK_HEADERS([portaudio.h], [], [have_portaudio2=no])
fi

if test "x${have_portaudio2}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The portaudio2 library and header file required to build ghpsdr3.
-----------------------------------])

fi



# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT32_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor ftime getcwd gethostbyname gettimeofday inet_ntoa memset pow select socket sqrt strrchr strstr strtol])


AC_CONFIG_SUBDIRS([trunk/src/QtRadio])
AC_CONFIG_FILES([Makefile
                 trunk/src/softrock/Makefile
                 trunk/src/DttSP/Makefile
                 trunk/src/dspserver/Makefile])



AC_OUTPUT

echo \
"-----------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'.
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
Now type 'make @<:@<target>@:>@'

 where the otpional <target> is:

    all           - build all binaries
    install       - install everything

-----------------------------------------------------"
