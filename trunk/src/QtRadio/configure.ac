AC_INIT([QtRadio Build], [beta], [andrew att montefusco dott com], [QtRadio])

#AC_CONFIG_AUX_DIR([build-aux])
#AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_MACRO_DIR([build-aux])

AM_INIT_AUTOMAKE([foreign -Wall])

## -------------------- ##
## Tool/Compilers setup ##
## -------------------- ##

# Look for a C++ compiler.
AC_LANG([C++])
AC_PROG_CXX

# If you need to create a library, USE libtool:
AC_PROG_LIBTOOL

# If you happen to have some C code:
#AC_LANG([C])
#AC_PROG_CC

# QtRadio with Qt.
AT_WITH_QT([core gui network])

AC_CONFIG_FILES([
    Makefile
])

AC_OUTPUT
