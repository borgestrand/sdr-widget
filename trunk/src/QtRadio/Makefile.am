ACLOCAL_AMFLAGS = -I build-aux

# Manually list the files you're using from build-aux here.
# Note: Automake already takes care of the .mk and .m4 files you use.
include $(srcdir)/build-aux/autotroll.mk


QtRadio_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
QtRadio_CPPFLAGS = $(QT_ADDITIONAL_INCLUDE_PATH) $(QT_CPPFLAGS) $(AM_CPPFLAGS) -DGITREV="\"$(GITSTR)\""

QtRadio_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(QT_ADDITIONAL_LDFLAG) -lQtOpenGL  -lcodec2  
QtRadio_LDADD    = $(QT_LIBS) $(LDADD)

# For a program:
bin_PROGRAMS = QtRadio

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
BandLimit.h  BookmarksDialog.h  Configure.h            DIGUFilters.h  FMNFilters.h   Meter.h          USBFilters.h  smeter.h \
About.ui  Bandscope.ui  BookmarksDialog.ui  BookmarksEditDialog.ui  Bookmark.ui  Configure.ui  KeypadDialog.ui  UI.ui  vfo.ui

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

#
# auxiliary services, git string, icons
#


desktop_in_files = QtRadio.desktop.in
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)



EXTRA_DIST = GITSTR  icons $(desktop_in_files) $(desktop_DATA)

pixmapdir = $(datadir)/icons
pixmap_DATA = $(srcdir)/icons/QtRadio.png


UPDATE_DESKTOP = \
	if which update-desktop-database>/dev/null 2>&1 ; then \
		update-desktop-database; \
	fi

install-data-hook:
	xdg-desktop-menu install --mode user --novendor $(srcdir)/QtRadio.desktop

uninstall-hook:
	xdg-desktop-menu uninstall --mode user --novendor QtRadio.desktop

