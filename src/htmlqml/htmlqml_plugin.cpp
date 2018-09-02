/* Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
   Use of this source code is governed by a Apache license that can be
   found in the LICENSE file.
*/

#include <QtQml>

#include "htmlqml_plugin.h"

#include <qhtmlpart.h>
#include <qhtmlview.h>

QML_DECLARE_TYPE(KParts::BrowserExtension)

class QtHtmlQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QtHtmlQmlPlugin(QObject *parent = 0) : QQmlExtensionPlugin(parent) { }
    virtual void registerTypes(const char *uri)
    {
        registerHtmlQmlTypes();
    }

};

void registerHtmlQmlTypes()
{
    const char *uri = "QtHtmlQml";

    Q_ASSERT(QLatin1String(uri) == QLatin1String("QtHtmlQml"));

    // QtHtmlQml 1.0
    qmlRegisterType<QHTMLPart>(uri, 1, 0, "HTMLPart");
    qmlRegisterType<QHTMLView>(uri, 1, 0, "HTMLView");

    qmlRegisterType<KParts::BrowserExtension>(uri, 1, 0, "BrowserExtension");
}

#include "htmlqml_plugin.moc"
