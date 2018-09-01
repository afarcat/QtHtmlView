/* Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
   Use of this source code is governed by a Apache license that can be
   found in the LICENSE file.
*/

#include "htmlbarwidget.h"
#include "ui_htmlbarwidget.h"

#include "htmlwidget.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QStandardPaths>

HtmlBarWidget::HtmlBarWidget(HtmlWidget *parentHtmlWidget, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HtmlBarWidget)
{
    ui->setupUi(this);

    htmlWidget = parentHtmlWidget;

    canAddPrev = true;

    connect(htmlWidget->browserExtension(), SIGNAL(openUrlRequest(QUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)),
            this, SLOT(slotOpenURL(QUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)));

    connect(htmlWidget, SIGNAL(urlChanged(const QUrl &, const QUrl &)), this, SLOT(slotUrlChanged(const QUrl &, const QUrl &)));
    connect(ui->toolButtonPrev, SIGNAL(clicked()), this, SLOT(slotPrevUrl()));
    connect(ui->toolButtonNext, SIGNAL(clicked()), this, SLOT(slotNextUrl()));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGotoUrl(int)));
    connect(ui->toolButtonOpen, SIGNAL(clicked()), this, SLOT(slotOpenUrl()));

    ui->comboBox->addItem("http://www.qtcn.org/bbs/i.php");
    ui->comboBox->addItem("http://blog.qt.io/");

    updateButton();
}

HtmlBarWidget::~HtmlBarWidget()
{
    delete ui;
}

void HtmlBarWidget::updateButton()
{
    ui->toolButtonPrev->setEnabled(!prevUrls.isEmpty());
    ui->toolButtonNext->setEnabled(!nextUrls.isEmpty());
}

void HtmlBarWidget::slotOpenURL(const QUrl &url, const KParts::OpenUrlArguments &args, const KParts::BrowserArguments &browserArgs)
{
    htmlWidget->setArguments(args);
    htmlWidget->browserExtension()->setBrowserArguments(browserArgs);
    htmlWidget->openUrl(url);
}

void HtmlBarWidget::slotUrlChanged(const QUrl &url, const QUrl &prevUrl)
{
    //
    QString item = url.toDisplayString();

    ui->comboBox->blockSignals(true);

    int index = ui->comboBox->findText(item);
    if (index != -1) {
        ui->comboBox->removeItem(index);
    }

    ui->comboBox->insertItem(0, item);
    ui->comboBox->setCurrentIndex(0);

    ui->comboBox->blockSignals(false);

    //
    if (canAddPrev && prevUrl.isValid()) {
        qDebug() << "prevUrls enqueue url=" << prevUrl;

        prevUrls.enqueue(prevUrl);

        updateButton();
    }

    canAddPrev = true;
}

void HtmlBarWidget::slotPrevUrl()
{
    if (!prevUrls.isEmpty()) {
        QUrl url1 = prevUrls.dequeue();

        QUrl url2 = htmlWidget->url();
        if (url2.isValid()) {
            qDebug() << "nextUrls enqueue url=" << url2;

            nextUrls.enqueue(url2);
        }

        canAddPrev = false;

        htmlWidget->openUrl(url1);

        updateButton();
    }
}

void HtmlBarWidget::slotNextUrl()
{
    if (!nextUrls.isEmpty()) {
        QUrl url = nextUrls.dequeue();

        htmlWidget->openUrl(url);

        updateButton();
    }
}

void HtmlBarWidget::slotGotoUrl(int index)
{
    QUrl url = QUrl::fromUserInput(ui->comboBox->itemText(index));
    htmlWidget->openUrl(url);
}

void HtmlBarWidget::slotOpenUrl()
{
    QUrl url = QFileDialog::getOpenFileUrl(htmlWidget->widget(),
                                           tr("Please select file"),
                                           QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).last(),
                                           QString("*.htm *.html"));
    if (url.isValid()) {
        htmlWidget->openUrl(url);
    }
}
