# Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
#   Use of this source code is governed by a Apache license that can be
#   found in the LICENSE file.

#NO java kjs svg ssl
#NO form audio video

#CONFIG += ENABLE_KJS ENABLE_ECMA ENABLE_DOM ENABLE_DOM2 ENABLE_DOM3
#DEFINES += ENABLE_KJS ENABLE_ECMA ENABLE_DOM ENABLE_DOM2 ENABLE_DOM3

DEFINES += KHTML_LIBRARY

msvc {
    DEFINES += _USE_MATH_DEFINES

    CONFIG += ENABLE_DOM ENABLE_DOM2
    DEFINES +=ENABLE_DOM ENABLE_DOM2
}

gcc {
    QMAKE_CXXFLAGS += -Wno-expansion-to-defined
}

#make sure this at first
INCLUDEPATH += $$PWD/compat $$PWD/source

include (3rdparty/kde.pri)
include (3rdparty/khtml.pri)
include (3rdparty/kjs.pri)

#qhtml
HEADERS += \
    $$PWD/source/qhtmlchildframe_p.h \
    $$PWD/source/qhtmlevents.h \
    $$PWD/source/qhtmlfilter_p.h \
    $$PWD/source/qhtmlglobal.h \
    $$PWD/source/qhtmlpart.h \
    $$PWD/source/qhtmlpart_p.h \
    $$PWD/source/qhtmlsettings.h \
    $$PWD/source/qhtmlview.h

SOURCES += \
    $$PWD/source/qhtmlchildframe.cpp \
    $$PWD/source/qhtmlevents.cpp \
    $$PWD/source/qhtmlfilter.cpp \
    $$PWD/source/qhtmlglobal.cpp \
    $$PWD/source/qhtmlpart.cpp \
    $$PWD/source/qhtmlsettings.cpp \
    $$PWD/source/qhtmlview.cpp
