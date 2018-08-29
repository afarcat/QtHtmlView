/* Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
   Use of this source code is governed by a Apache license that can be
   found in the LICENSE file.
*/

#ifndef HTMLBARWIDGET_H
#define HTMLBARWIDGET_H

#include <QWidget>
#include <QQueue>

#include <kparts/browserextension.h>
#include <kparts/browserarguments.h>
#include <kparts/openurlarguments.h>

namespace Ui {
class HtmlBarWidget;
}

class HtmlWidget;
class HtmlBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HtmlBarWidget(HtmlWidget *parentHtmlWidget, QWidget *parent = nullptr);
    ~HtmlBarWidget();

private:
    void updateButton();

private slots:
    void slotOpenURL(const QUrl &url, const KParts::OpenUrlArguments &args, const KParts::BrowserArguments &browserArgs);
    void slotUrlChanged(const QUrl &url, const QUrl &prevUrl);
    void slotPrevUrl();
    void slotNextUrl();
    void slotGotoUrl(int index);
    void slotOpenUrl();

private:
    Ui::HtmlBarWidget *ui;
    HtmlWidget *htmlWidget;

    bool canAddPrev;
    QQueue<QUrl> prevUrls;
    QQueue<QUrl> nextUrls;
};

#endif // HTMLBARWIDGET_H
