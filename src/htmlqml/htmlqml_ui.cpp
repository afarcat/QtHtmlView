/* Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
   Use of this source code is governed by a Apache license that can be
   found in the LICENSE file.
*/

#include "htmlqml_ui.h"

#include "qhtmlpart.h"
#include "qhtmlview.h"

using namespace khtml;

#ifndef QT_WIDGETS_LIB

// -----------------------------------------------------------------------------
QQmlWidget::QQmlWidget(QWidget *parent, QString className)
    : QWidget(parent)
{
    QHTMLView *htmlView = qobject_cast<QHTMLView *>(parent);
    if (htmlView && htmlView->part()) {
        QVariant returnArg;
        if (QMetaObject::invokeMethod(htmlView->part(), "createQmlWidget", Qt::DirectConnection,
                                      Q_RETURN_ARG(QVariant, returnArg),
                                      Q_ARG(QVariant, QVariant::fromValue(parent)),
                                      Q_ARG(QVariant, className))){
            m_qmlWidget = returnArg.value<QWidget *>();
        }
    }

    QObject::connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotAppQuit()));
}

QQmlWidget::~QQmlWidget()
{
    if (!m_qmlWidget.isNull()) {
        QHTMLView *htmlView = qobject_cast<QHTMLView *>(parent());
        if (htmlView && htmlView->part()) {
            QMetaObject::invokeMethod(htmlView->part(), "destroyQmlWidget", Qt::DirectConnection,
                                      Q_ARG(QVariant, QVariant::fromValue(m_qmlWidget.data())));
            m_qmlWidget = nullptr;
        }
    }
}

QWidget *QQmlWidget::qmlWidget() const
{
    return m_qmlWidget;
}

void QQmlWidget::setQmlWidget(QWidget *qmlWidget)
{
    m_qmlWidget = qmlWidget;
}

void QQmlWidget::slotAppQuit()
{
    //set m_qmlWidget to nullptr to prevent call ~QQmlWidget()
    m_qmlWidget = nullptr;
}

// -----------------------------------------------------------------------------
QCheckBox::QCheckBox(QWidget *parent)
    : QQmlWidget(parent, "QCheckBox")
{
    if (m_qmlWidget) {
        connect(m_qmlWidget, SIGNAL(checkedChanged()), this, SLOT(slotCheckedChanged()));
    }
}

bool QCheckBox::isChecked() const
{
    if (m_qmlWidget) {
        return m_qmlWidget->property("checked").toBool();
    }
    return false;
}

void QCheckBox::setChecked(bool value)
{
    if (m_qmlWidget) {
        m_qmlWidget->setProperty("checked", value);
    }
}

void QCheckBox::slotCheckedChanged()
{
    emit stateChanged(m_qmlWidget->property("checkState").toInt());
}

// -----------------------------------------------------------------------------
QComboBox::QComboBox(QWidget *parent)
    : QQmlWidget(parent, "QComboBox")
{

}

// -----------------------------------------------------------------------------
QRadioButton::QRadioButton(QWidget *parent)
    : QQmlWidget(parent, "QRadioButton")
{

}

void QRadioButton::setAutoExclusive(bool exclusive)
{
    if (m_qmlWidget) {
        m_qmlWidget->setProperty("autoExclusive", exclusive);
    }
}

void QRadioButton::setChecked(bool checked)
{
    if (m_qmlWidget) {
        m_qmlWidget->setProperty("checked", checked);
    }
}

// -----------------------------------------------------------------------------
QPushButton::QPushButton(QWidget *parent)
    : QQmlWidget(parent, "QPushButton")
{

}

// -----------------------------------------------------------------------------
QLineEdit::QLineEdit(QWidget *parent)
    : QQmlWidget(parent, "QLineEdit")
{

}

void QLineEdit::setEchoMode(QQuickTextInput::EchoMode echo)
{
    if (m_qmlWidget) {
        m_qmlWidget->setProperty("echoMode", echo);
    }
}

// -----------------------------------------------------------------------------
QTextEdit::QTextEdit(QWidget *parent)
    : QQmlWidget(parent, "QTextEdit")
{

}

void QTextEdit::setReadOnly(bool v)
{
    if (m_qmlWidget) {
        m_qmlWidget->setProperty("readOnly", v);
    }
}

void QTextEdit::setWrapMode(QQuickTextEdit::WrapMode w)
{
    if (m_qmlWidget) {
        m_qmlWidget->setProperty("wrapMode", w);
    }
}

void QTextEdit::setTextFormat(QQuickTextEdit::TextFormat format)
{
    if (m_qmlWidget) {
        m_qmlWidget->setProperty("textFormat", format);
    }
}

// -----------------------------------------------------------------------------
QListWidget::QListWidget(QWidget *parent)
    : QQmlWidget(parent, "QListWidget")
{

}

// -----------------------------------------------------------------------------
QScrollBar::QScrollBar(QWidget *parent)
    : QQmlWidget(parent, "QScrollBar")
{

}

void QScrollBar::setOrientation(Qt::Orientation orientation)
{
    if (m_qmlWidget) {
        m_qmlWidget->setProperty("orientation", orientation);
    }
}

qreal QScrollBar::size() const
{
    if (m_qmlWidget) {
        m_qmlWidget->property("size").toReal();
    }
    return 0.0;
}

qreal QScrollBar::position() const
{
    if (m_qmlWidget) {
        m_qmlWidget->property("position").toReal();
    }
    return 0.0;
}

void QScrollBar::setPosition(qreal position)
{
    if (m_qmlWidget) {
        QMetaObject::invokeMethod(m_qmlWidget, "setPosition", Q_ARG(QVariant, QVariant::fromValue(position)));
    }
}

void QScrollBar::increase()
{
    if (m_qmlWidget) {
        QMetaObject::invokeMethod(m_qmlWidget, "increase");
    }
}

void QScrollBar::decrease()
{
    if (m_qmlWidget) {
        QMetaObject::invokeMethod(m_qmlWidget, "decrease");
    }
}

#endif
