# Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
#   Use of this source code is governed by a Apache license that can be
#   found in the LICENSE file.

TARGET = HtmlWidget
TEMPLATE = app

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../src/compat ../../src/source
INCLUDEPATH += ../../src/3rdparty/kde ../../src/3rdparty/kde/kio
INCLUDEPATH += ../../src/3rdparty/khtml/src
INCLUDEPATH += ../../src/3rdparty/kjs/src ../../src/3rdparty/kjs/src/kjs

DESTDIR = ../../bin

LIBS += -L$$DESTDIR
LIBS += -lQtHtmlView

SOURCES += \
    main.cpp \
    htmlwidget.cpp \
    htmlbarwidget.cpp

HEADERS += \
    htmlwidget.h \
    htmlbarwidget.h

FORMS += \
    htmlbarwidget.ui

RESOURCES += \
    htmlwidget.qrc
