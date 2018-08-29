/* Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
   Use of this source code is governed by a Apache license that can be
   found in the LICENSE file.
*/

#ifndef HTMLWIDGET_H
#define HTMLWIDGET_H

#include "qhtmlpart.h"
#include "htmlbarwidget.h"

class HtmlWidget : public QHTMLPart
{
    Q_OBJECT
public:
    explicit HtmlWidget(QWidget *parentWidget = nullptr, QObject *parent = nullptr);

public slots:
    void slotOnUrl(const QString &url);

private:
    HtmlBarWidget *htmlBarWidget;
};

#endif // HTMLWIDGET_H
