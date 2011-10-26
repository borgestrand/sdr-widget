#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ghpsdr3], [1.5], [andrew@montefusco.com])

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

#
# Checks for libraries.
# Check both libraries binary and header files
#

have_pthreads=no
AC_SEARCH_LIBS([pthread_create], [pthread],[have_pthreads=yes])

have_portaudio2=no
AC_SEARCH_LIBS([Pa_Initialize], [portaudio],[have_portaudio2=yes])

have_fftw3=no
AC_SEARCH_LIBS([fftwf_malloc], [fftw3f],[have_fftw3=yes])

have_codec2=no
AC_SEARCH_LIBS([codec2_create], [codec2],[have_codec2=yes])

have_pulse_simple=no
AC_SEARCH_LIBS([pa_simple_new], [pulse-simple],[have_pulse_simple=yes])

have_usb=no
AC_SEARCH_LIBS([usb_init], [usb],[have_usb=yes])

have_jack=no
AC_SEARCH_LIBS([jack_client_open], [jack],[have_jack=yes])


# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h sys/timeb.h unistd.h])

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


if test "x${have_fftw3}" = xyes; then
    AC_CHECK_HEADERS([fftw3.h], [], [have_fftw3=no])
fi

if test "x${have_fftw3}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The fftw3 library and header file required to build ghpsdr3.
-----------------------------------])

fi


if test "x${have_codec2}" = xyes; then
    AC_CHECK_HEADERS([codec2.h], [], [have_codec2=no])
fi

if test "x${have_codec2}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The codec2 library and header file required to build ghpsdr3.
-----------------------------------])

fi


if test "x${have_pulse_simple}" = xyes; then
    AC_CHECK_HEADERS([pulse/simple.h], [], [have_pulse_simple=no])
fi

if test "x${have_pulse_simple}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The pulse-simple library and header file required to build ghpsdr3.
-----------------------------------])

fi


if test "x${have_usb}" = xyes; then
    AC_CHECK_HEADERS([usb.h], [], [have_usb=no])
fi

if test "x${have_usb}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The libusb-0.1 library and header file required to build ghpsdr3.
-----------------------------------])

fi

if test "x${have_jack}" = xyes; then
    AC_CHECK_HEADERS([jack/jack.h], [], [have_jack=no])
fi

if test "x${have_jack}" = xno; then
    AC_MSG_ERROR([
-----------------------------------
  The libjack library and header file required to build ghpsdr3.
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
                 trunk/src/dspserver/Makefile
                 trunk/src/widget-server/Makefile])



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
