/* Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
   Use of this source code is governed by a Apache license that can be
   found in the LICENSE file.
*/

#include <QApplication>
#include <QFile>
#include <QDesktopWidget>

#include "htmlwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    HtmlWidget htmlWidget;
    htmlWidget.show();

    QDesktopWidget *desktopWidget = QApplication::desktop();
    htmlWidget.widget()->resize(desktopWidget->width()*0.75, desktopWidget->height()*0.75);
    htmlWidget.widget()->move(desktopWidget->width()*0.125, desktopWidget->height()*0.125);

    htmlWidget.setCaretMode(true);
    htmlWidget.setCaretVisible(true);

    return a.exec();
}
