# Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
#   Use of this source code is governed by a Apache license that can be
#   found in the LICENSE file.

TARGET = QtHtmlQml
TEMPLATE = lib

QT += core gui xml multimedia network
QT += quick quickcontrols2
QT += qml-private quick-private quicktemplates2-private

DESTDIR = ../../bin

include (../src.pri)

SOURCES += \
    $$PWD/htmlqml_plugin.cpp
