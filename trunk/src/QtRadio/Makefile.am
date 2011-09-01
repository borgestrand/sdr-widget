ACLOCAL_AMFLAGS = -I build-aux

#SUBDIRS = src gui tests
#EXTRA_DIST = bootstrap

# Manually list the files you're using from build-aux here.
# Note: Automake already takes care of the .mk and .m4 files you use.
#EXTRA_DIST += \
#		tests/test.sh

include $(top_srcdir)/build-aux/autotroll.mk

QtRadio_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
QtRadio_CPPFLAGS = $(QT_ADDITIONAL_INCLUDE_PATH) $(QT_CPPFLAGS) $(AM_CPPFLAGS) 
########## -I$(QT_INCPATH)/include/QtWebKit  -I$(QT_INCPATH)/include/QtMultimediaKit

QtRadio_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(QT_ADDITIONAL_LDFLAG) -lQtOpenGL  -lcodec2  
QtRadio_LDADD    = $(QT_LIBS) $(LDADD)

# For a program:
bin_PROGRAMS = QtRadio

nodist_QtRadio_SOURCES = \
    ui_UI.h \
    ui_Configure.h \
    ui_Bandscope.h \
    ui_About.h \
    ui_Bookmark.h \
    ui_BookmarksDialog.h \
    ui_BookmarksEditDialog.h \
    ui_KeypadDialog.h \
    ui_vfo.h \
About.moc.cpp                  \
Audio.moc.cpp                  \
Band.moc.cpp                   \
Bandscope.moc.cpp              \
BandStackEntry.moc.cpp         \
Bookmark.moc.cpp               \
BookmarkDialog.moc.cpp         \
Bookmarks.moc.cpp              \
BookmarksDialog.moc.cpp        \
BookmarksEditDialog.moc.cpp    \
Configure.moc.cpp              \
Connection.moc.cpp             \
Filter.moc.cpp                 \
FiltersBase.moc.cpp            \
Filters.moc.cpp                \
KeypadDialog.moc.cpp           \
Mode.moc.cpp                   \
Spectrum.moc.cpp               \
UI.moc.cpp                     \
vfo.moc.cpp                    \
Waterfall.moc.cpp              \
Xvtr.moc.cpp                   \
smeter.moc.cpp                 




QtRadio_SOURCES = \
About.cpp      BandStackEntry.cpp       Buffer.cpp       DIGUFilters.cpp  Frequency.cpp      Mode.cpp        Waterfall.cpp  \
AMFilters.cpp  Bookmark.cpp             Configure.cpp    DSBFilters.cpp   FrequencyInfo.cpp  SAMFilters.cpp  Xvtr.cpp       \
Audio.cpp      BookmarkDialog.cpp       Connection.cpp   Filter.cpp       KeypadDialog.cpp   Spectrum.cpp    XvtrEntry.cpp  \
Band.cpp       Bookmarks.cpp            CWLFilters.cpp   FiltersBase.cpp  LSBFilters.cpp     UI.cpp                         \
BandLimit.cpp  BookmarksDialog.cpp      CWUFilters.cpp   Filters.cpp      main.cpp           USBFilters.cpp                 \
Bandscope.cpp  BookmarksEditDialog.cpp  DIGLFilters.cpp  FMNFilters.cpp   Meter.cpp          vfo.cpp         smeter.cpp     \
About.h      Bandscope.h        BookmarksEditDialog.h  Connection.h   DSBFilters.h   Frequency.h      Mode.h        vfo.h        \
AMFilters.h  BandStackEntry.h   Bookmarks.h            CWLFilters.h   Filter.h       FrequencyInfo.h  SAMFilters.h  Waterfall.h  \
Audio.h      BookmarkDialog.h   Buffer.h               CWUFilters.h   FiltersBase.h  KeypadDialog.h   Spectrum.h    XvtrEntry.h  \
Band.h       Bookmark.h         codec2.h               DIGLFilters.h  Filters.h      LSBFilters.h     UI.h          Xvtr.h       \
BandLimit.h  BookmarksDialog.h  Configure.h            DIGUFilters.h  FMNFilters.h   Meter.h          USBFilters.h  smeter.h     \
    ui_UI.h \
    ui_Configure.h \
    ui_Bandscope.h \
    ui_About.h \
    ui_Bookmark.h \
    ui_BookmarksDialog.h \
    ui_BookmarksEditDialog.h \
    ui_KeypadDialog.h \
    ui_vfo.h


BUILT_SOURCES=                  \
    ui_UI.h \
    ui_Configure.h \
    ui_Bandscope.h \
    ui_About.h \
    ui_Bookmark.h \
    ui_BookmarksDialog.h \
    ui_BookmarksEditDialog.h \
    ui_KeypadDialog.h \
    ui_vfo.h \
About.moc.cpp                    \
Audio.moc.cpp                    \
Band.moc.cpp                     \
Bandscope.moc.cpp                \
BandStackEntry.moc.cpp           \
Bookmark.moc.cpp                 \
BookmarkDialog.moc.cpp           \
Bookmarks.moc.cpp                \
BookmarksDialog.moc.cpp          \
BookmarksEditDialog.moc.cpp      \
Configure.moc.cpp                \
Connection.moc.cpp               \
Filter.moc.cpp                   \
FiltersBase.moc.cpp              \
Filters.moc.cpp                  \
KeypadDialog.moc.cpp             \
Mode.moc.cpp                     \
Spectrum.moc.cpp                 \
UI.moc.cpp                       \
vfo.moc.cpp                      \
Waterfall.moc.cpp                \
Xvtr.moc.cpp \
smeter.moc.cpp 


CLEANFILES =                  \
    ui_UI.h \
    ui_Configure.h \
    ui_Bandscope.h \
    ui_About.h \
    ui_Bookmark.h \
    ui_BookmarksDialog.h \
    ui_BookmarksEditDialog.h \
    ui_KeypadDialog.h \
    ui_vfo.h \
About.moc.cpp                    \
Audio.moc.cpp                    \
Band.moc.cpp                     \
Bandscope.moc.cpp                \
BandStackEntry.moc.cpp           \
Bookmark.moc.cpp                 \
BookmarkDialog.moc.cpp           \
Bookmarks.moc.cpp                \
BookmarksDialog.moc.cpp          \
BookmarksEditDialog.moc.cpp      \
Configure.moc.cpp                \
Connection.moc.cpp               \
Filter.moc.cpp                   \
FiltersBase.moc.cpp              \
Filters.moc.cpp                  \
KeypadDialog.moc.cpp             \
Mode.moc.cpp                     \
Spectrum.moc.cpp                 \
UI.moc.cpp                       \
vfo.moc.cpp                      \
Waterfall.moc.cpp                \
Xvtr.moc.cpp                     \
smeter.moc.cpp                   


