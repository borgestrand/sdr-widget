AC_INIT([QtRadio Build], [1.3], [andrew att montefusco dott com], [QtRadio])

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

#
# git repository parameters copied in GITSTR in order to display in "About" panel
#
GITSTR=`(git show -s --pretty=format:"%h!%s" 2>&1)`"!"`(git branch --no-color 2>&1  | sed -e '/^[[^*]]/d' -e 's/* \(.*\)/\1/')`
if { ! ( git show ) 1>/dev/null 2>&1 || test "$GITSTR" = "fatal"; } ;
   then GITSTR=`cat $srcdir/GITSTR`
   else echo $GITSTR>$srcdir/GITSTR
fi
AC_SUBST(GITSTR)


# QtRadio with Qt.
AT_WITH_QT([core gui network])
AT_REQUIRE_QT_VERSION(4.0.0, AC_MSG_ERROR([Bad Qt: 4.x required !]))

AC_MSG_RESULT([QT version: $QT_VERSION])

AC_MSG_RESULT([QT_LIBS: $QT_LIBS])

AC_MSG_RESULT([QT_PATH: $QT_PATH])

AC_MSG_RESULT([QT_INCPATH: $QT_INCPATH])

AC_MSG_RESULT([QT_INSTALL_HEADERS: $QT_INSTALL_HEADERS])
AC_MSG_RESULT([QT_INSTALL_HEADERS_UP: $QT_INSTALL_HEADERS_UP])

AC_MSG_RESULT([QT_ADDITIONAL_INCLUDE_PATH: $QT_ADDITIONAL_INCLUDE_PATH])
AC_MSG_RESULT([QT_ADDITIONAL_LDFLAG: $QT_ADDITIONAL_LDFLAG])


AC_CONFIG_FILES([
    Makefile
])

AC_OUTPUT
