/* Copyright (C) 2018 afarcat <kabak@sina.com>. All rights reserved.
   Use of this source code is governed by a Apache license that can be
   found in the LICENSE file.
*/

#ifndef __htmlqmlui_h
#define __htmlqmlui_h

#ifndef QT_WIDGETS_LIB

#include <QPointer>
#include <QQuickItem>
#define QWidget QQuickItem

#include <QtQuickTemplates2/private/qquickcombobox_p.h>
#include <QtQuickTemplates2/private/qquickcheckbox_p.h>
#include <QtQuickTemplates2/private/qquickradiobutton_p.h>
#include <QtQuickTemplates2/private/qquickbutton_p.h>
#include <QtQuickTemplates2/private/qquicktextfield_p.h>
#include <QtQuickTemplates2/private/qquicktextarea_p.h>
#include <QtQuickTemplates2/private/qquickscrollbar_p.h>
#include <QtQuick/private/qquicklistview_p.h>

//AFA: QtQuickTemplates2 NO UI, so use it in qml file
//#define QCheckBox QQuickCheckBox
//#define QComboBox QQuickComboBox
//#define QRadioButton QQuickRadioButton
//#define QPushButton QQuickButton
//#define QLineEdit QQuickTextField
//#define QTextEdit QQuickTextArea
//#define QListWidget QQuickListView
//#define QScrollBar QQuickScrollBar

// -----------------------------------------------------------------------------
class QQmlWidget : public QWidget
{
    Q_OBJECT
public:
    QQmlWidget(QWidget *parent = 0, QString className = "");
    ~QQmlWidget();

public:
    QWidget *qmlWidget() const;
    void setQmlWidget(QWidget *qmlWidget);

protected slots:
    void slotAppQuit();

protected:
    QPointer<QWidget> m_qmlWidget;
};

// -----------------------------------------------------------------------------
class QCheckBox : public QQmlWidget
{
    Q_OBJECT
public:
    QCheckBox(QWidget * parent = 0);

public:
    bool isChecked() const;
    void setChecked(bool value);

Q_SIGNALS:
    void stateChanged(int);

protected slots:
    void slotCheckedChanged();
};

// -----------------------------------------------------------------------------
class QComboBox : public QQmlWidget
{
    Q_OBJECT
public:
    QComboBox(QWidget * parent = 0);
};

// -----------------------------------------------------------------------------
class QRadioButton : public QQmlWidget
{
    Q_OBJECT
public:
    QRadioButton(QWidget * parent = 0);

public:
    void setAutoExclusive(bool exclusive);
    void setChecked(bool checked);
};

// -----------------------------------------------------------------------------
class QPushButton : public QQmlWidget
{
    Q_OBJECT
public:
    QPushButton(QWidget * parent = 0);
};

// -----------------------------------------------------------------------------
class QLineEdit : public QQmlWidget
{
    Q_OBJECT
public:
    QLineEdit(QWidget * parent = 0);

public:
    void setEchoMode(QQuickTextInput::EchoMode echo);
};

// -----------------------------------------------------------------------------
class QTextEdit : public QQmlWidget
{
    Q_OBJECT
public:
    QTextEdit(QWidget * parent = 0);

public:
    void setReadOnly(bool);
    void setWrapMode(QQuickTextEdit::WrapMode w);
    void setTextFormat(QQuickTextEdit::TextFormat format);
};

// -----------------------------------------------------------------------------
class QListWidget : public QQmlWidget
{
    Q_OBJECT
public:
    QListWidget(QWidget * parent = 0);
};

// -----------------------------------------------------------------------------
class QScrollBar : public QQmlWidget
{
    Q_OBJECT
public:
    QScrollBar(QWidget * parent = 0);

public:
    void setOrientation(Qt::Orientation orientation);
    qreal size() const;
    qreal position() const;
    void setPosition(qreal position);
    void increase();
    void decrease();
};

#endif

#endif // __htmlqmlui_h
