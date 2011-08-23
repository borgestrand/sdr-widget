#-------------------------------------------------
#
# Project created by QtCreator 2010-07-05T10:00:09
#
#-------------------------------------------------

//QT       += core gui network multimedia
//QT	+= mobility multimediakit
CONFIG	+= mobility
MOBILITY += multimedia

INCLUDEPATH += /usr/include/QtMobility
INCLUDEPATH += /usr/include/QtMultimediaKit

TARGET = QtRadio
TEMPLATE = app


SOURCES += main.cpp\
    Waterfall.cpp \
    USBFilters.cpp \
    UI.cpp \
    Spectrum.cpp \
    SAMFilters.cpp \
    Mode.cpp \
    LSBFilters.cpp \
    FMNFilters.cpp \
    FiltersBase.cpp \
    Filters.cpp \
    Filter.cpp \
    DSBFilters.cpp \
    DIGUFilters.cpp \
    DIGLFilters.cpp \
    CWUFilters.cpp \
    CWLFilters.cpp \
    Connection.cpp \
    Configure.cpp \
    BandStackEntry.cpp \
    Band.cpp \
    Audio.cpp \
    AMFilters.cpp \
    BandLimit.cpp \
    FrequencyInfo.cpp \
    Frequency.cpp \
    Meter.cpp \
    Bandscope.cpp \
    About.cpp \
    Buffer.cpp \
    Bookmark.cpp \
    BookmarkDialog.cpp \
    BookmarksDialog.cpp \
    BookmarksEditDialog.cpp \
    Xvtr.cpp \
    XvtrEntry.cpp \
    Bookmarks.cpp \
    KeypadDialog.cpp \
    vfo.cpp
        

HEADERS  += \ 
    Waterfall.h \
    USBFilters.h \
    UI.h \
    Spectrum.h \
    SAMFilters.h \
    Mode.h \
    LSBFilters.h \
    FMNFilters.h \
    FiltersBase.h \
    Filters.h \
    Filter.h \
    DSBFilters.h \
    DIGUFilters.h \
    DIGLFilters.h \
    CWUFilters.h \
    CWLFilters.h \
    Connection.h \
    Configure.h \
    BandStackEntry.h \
    Band.h \
    Audio.h \
    AMFilters.h \
    BandLimit.h \
    FrequencyInfo.h \
    Frequency.h \
    Meter.h \
    Bandscope.h \
    About.h \
    Buffer.h \
    Bookmark.h \
    BookmarkDialog.h \
    BookmarksDialog.h \
    BookmarksEditDialog.h \
    Xvtr.h \
    XvtrEntry.h \
    Bookmarks.h \
    KeypadDialog.h \
    codec2.h \
    vfo.h

FORMS    += \   
    UI.ui \
    Configure.ui \
    Bandscope.ui \
    About.ui \
    Bookmark.ui \
    BookmarksDialog.ui \
    BookmarksEditDialog.ui \
    KeypadDialog.ui \
    vfo.ui

OTHER_FILES +=

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../usr/lib/release/ -lcodec2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../usr/lib/debug/ -lcodec2
else:symbian: LIBS += -lcodec2
else:unix: LIBS += -lcodec2

