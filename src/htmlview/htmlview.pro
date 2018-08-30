# Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
#   Use of this source code is governed by a Apache license that can be
#   found in the LICENSE file.

TARGET = QtHtmlView
TEMPLATE = lib

QT += core gui xml multimedia network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DESTDIR = ../../bin

include (../src.pri)
