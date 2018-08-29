/* Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
   Use of this source code is governed by a Apache license that can be
   found in the LICENSE file.
*/

#include "htmlwidget.h"

#include <QToolTip>
#include <QBoxLayout>

HtmlWidget::HtmlWidget(QWidget *parentWidget, QObject *parent) : QHTMLPart(parentWidget, parent)
{
    htmlBarWidget = nullptr;

    QBoxLayout *layout = qobject_cast<QBoxLayout *>(widget()->layout());
    if (layout) {
        htmlBarWidget = new HtmlBarWidget(this, widget());
        layout->insertWidget(0, htmlBarWidget);
    }

    connect(this, SIGNAL(onURL(const QString &)), this, SLOT(slotOnUrl(const QString &)));
}

void HtmlWidget::slotOnUrl(const QString &url)
{
    if (url.isEmpty()) {
        QToolTip::hideText();
        return;
    }

    QPoint point = widget()->geometry().bottomLeft();
    point += QPoint(4, -48);
    QToolTip::showText(point, url, widget());
}
