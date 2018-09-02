# QtHtmlView - a fully functional HTHML viewer

## QtHtmlView

QtHtmlView is based on KHTML, The aim is to provide a Qt component with fast rendering speed, low memory footprint and small file size to display HTML.

1. Fully functional features of KHTML.

2. Only depend on Qt: Image decoder(include animation) and network library using Qt.

3. Small size: cutted KJS and SVG and other not necessary function.

4. Considering the compatibility of source code, the minimum function file of KPARTS and KIO Library of KDE is used.

## QtHtmlQml

QtHtmlQml is the QML implementation of QtHtmlView and does not depend on QtWidgets. It needs at least QtQuick.Controls 2.2 version support.

## Contributing Code

QtHtmlView is licensed same as KHTML. Please see the file COPYING for KHTML/KJS/KPARTS/KIO of licenses file.
The QtHtmlView example is licensed under Apache v2.0.

## Checking out the source

You can clone from github with

    git clone git@github.com:afarcat/QtHtmlView.git

or, if you are not a github user,

    git clone git://github.com/afarcat/QtHtmlView.git

## Build Prerequisites

To build QtHtmlView you need to install with:

1. Qt (>= 5.2)

2. C++ compiler with C11 function

3. perl

4. gperf

## Building QtHtmlView

Prerequisite: Qt Creator

Build instructions:

    In Qt Creator, open qthtmlview.pro, and compile it.
