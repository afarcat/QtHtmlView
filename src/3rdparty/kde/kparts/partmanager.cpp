/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>
   Copyright (C) 2018 afarcat <kabak@sina.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "partmanager.h"

//AFA #include "partactivateevent.h"
//AFA #include "partselectevent.h"
//AFA #include "guiactivateevent.h"
#include "part.h"

#include <QCoreApplication>
#ifdef QT_WIDGETS_LIB
#include <QScrollBar>
#endif
#include <QKeyEvent>

#include <QDebug>

//#define DEBUG_PARTMANAGER

using namespace KParts;

namespace KParts
{

class PartManagerPrivate
{
public:
    PartManagerPrivate()
    {
        m_activeWidget = nullptr;
        m_activePart = nullptr;
        m_selectedPart = nullptr;
        m_selectedWidget = nullptr;
        m_bAllowNestedParts = false;
        m_bIgnoreScrollBars = false;
        m_activationButtonMask = Qt::LeftButton | Qt::MidButton | Qt::RightButton;
        m_reason = PartManager::NoReason;
        m_bIgnoreExplicitFocusRequest = false;
    }
    ~PartManagerPrivate()
    {
    }
    void setReason(QEvent *ev)
    {
        switch (ev->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick: {
            QMouseEvent *mev = static_cast<QMouseEvent *>(ev);
            m_reason = mev->button() == Qt::LeftButton
                       ? PartManager::ReasonLeftClick
                       : (mev->button() == Qt::MidButton
                          ? PartManager::ReasonMidClick
                          : PartManager::ReasonRightClick);
            break;
        }
        case QEvent::FocusIn:
            m_reason = static_cast<QFocusEvent *>(ev)->reason();
            break;
        default:
            qWarning() << "PartManagerPrivate::setReason got unexpected ev type " << ev->type();
            break;
        }
    }

    bool allowExplicitFocusEvent(QEvent *ev) const
    {
        if (ev->type() == QEvent::FocusIn) {
            QFocusEvent *fev = static_cast<QFocusEvent *>(ev);
            return (!m_bIgnoreExplicitFocusRequest || fev->reason() != Qt::OtherFocusReason);
        }
        return true;
    }

    Part *m_activePart;
    QWidget *m_activeWidget;

    QList<Part *> m_parts;

    PartManager::SelectionPolicy m_policy;

    Part *m_selectedPart;
    QWidget *m_selectedWidget;

    QList<const QWidget *> m_managedTopLevelWidgets;
    short int m_activationButtonMask;
    bool m_bIgnoreScrollBars;
    bool m_bAllowNestedParts;
    int m_reason;
    bool m_bIgnoreExplicitFocusRequest;
};

}

PartManager::PartManager(QWidget *parent)
    : QObject(parent), d(new PartManagerPrivate)
{

    qApp->installEventFilter(this);

    d->m_policy = Direct;

    addManagedTopLevelWidget(parent);
}

PartManager::PartManager(QWidget *topLevel, QObject *parent)
    : QObject(parent), d(new PartManagerPrivate)
{

    qApp->installEventFilter(this);

    d->m_policy = Direct;

    addManagedTopLevelWidget(topLevel);
}

PartManager::~PartManager()
{
    foreach (const QWidget *w, d->m_managedTopLevelWidgets) {
        disconnect(w, &QWidget::destroyed,
                   this, &PartManager::slotManagedTopLevelWidgetDestroyed);
    }

    foreach (Part *it, d->m_parts) {
        it->setManager(nullptr);
    }

    // core dumps ... setActivePart( 0 );
    qApp->removeEventFilter(this);
    delete d;
}

void PartManager::setSelectionPolicy(SelectionPolicy policy)
{
    d->m_policy = policy;
}

PartManager::SelectionPolicy PartManager::selectionPolicy() const
{
    return d->m_policy;
}

void PartManager::setAllowNestedParts(bool allow)
{
    d->m_bAllowNestedParts = allow;
}

bool PartManager::allowNestedParts() const
{
    return d->m_bAllowNestedParts;
}

void PartManager::setIgnoreScrollBars(bool ignore)
{
    d->m_bIgnoreScrollBars = ignore;
}

bool PartManager::ignoreScrollBars() const
{
    return d->m_bIgnoreScrollBars;
}

void PartManager::setActivationButtonMask(short int buttonMask)
{
    d->m_activationButtonMask = buttonMask;
}

short int PartManager::activationButtonMask() const
{
    return d->m_activationButtonMask;
}

bool PartManager::eventFilter(QObject *obj, QEvent *ev)
{

    if (ev->type() != QEvent::MouseButtonPress &&
            ev->type() != QEvent::MouseButtonDblClick &&
            ev->type() != QEvent::FocusIn) {
        return false;
    }

    if (!obj->isWidgetType()) {
        return false;
    }

    QWidget *w = static_cast<QWidget *>(obj);
#ifdef QT_WIDGETS_LIB
    if (((w->windowFlags().testFlag(Qt::Dialog)) && w->isModal()) ||
            (w->windowFlags().testFlag(Qt::Popup)) || (w->windowFlags().testFlag(Qt::Tool))) {
        return false;
    }
#endif

    QMouseEvent *mev = nullptr;
    if (ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseButtonDblClick) {
        mev = static_cast<QMouseEvent *>(ev);
#ifdef DEBUG_PARTMANAGER
        qDebug() << "PartManager::eventFilter button: " << mev->button() << " " << "d->m_activationButtonMask=" << d->m_activationButtonMask;
#endif
        if ((mev->button() & d->m_activationButtonMask) == 0) {
            return false;    // ignore this button
        }
    }

    Part *part;
    while (w) {
        QPoint pos;
#ifdef QT_WIDGETS_LIB
        if (!d->m_managedTopLevelWidgets.contains(w->topLevelWidget())) {
            return false;
        }

        if (d->m_bIgnoreScrollBars && ::qobject_cast<QScrollBar *>(w)) {
            return false;
        }
#endif
        if (mev) { // mouse press or mouse double-click event
            pos = mev->globalPos();
            part = findPartFromWidget(w, pos);
        } else {
            part = findPartFromWidget(w);
        }

#ifdef DEBUG_PARTMANAGER
        const char *evType = (ev->type() == QEvent::MouseButtonPress) ? "MouseButtonPress"
                             : (ev->type() == QEvent::MouseButtonDblClick) ? "MouseButtonDblClick"
                             : (ev->type() == QEvent::FocusIn) ? "FocusIn" : "OTHER! ERROR!";
#endif
        if (part) { // We found a part whose widget is w
            if (d->m_policy == PartManager::TriState) {
                if (ev->type() == QEvent::MouseButtonDblClick) {
                    if (part == d->m_activePart && w == d->m_activeWidget) {
                        return false;
                    }

#ifdef DEBUG_PARTMANAGER
                    qDebug() << "PartManager::eventFilter dblclick -> setActivePart" << part;
#endif
                    d->setReason(ev);
                    setActivePart(part, w);
                    d->m_reason = NoReason;
                    return true;
                }

                if ((d->m_selectedWidget != w || d->m_selectedPart != part) &&
                        (d->m_activeWidget != w || d->m_activePart != part)) {
                    if (part->isSelectable()) {
                        setSelectedPart(part, w);
                    } else {
#ifdef DEBUG_PARTMANAGER
                        qDebug() << "Part " << part << " (non-selectable) made active because " << w->metaObject()->className() << " got event" << " " << evType;
#endif
                        d->setReason(ev);
                        setActivePart(part, w);
                        d->m_reason = NoReason;
                    }
                    return true;
                } else if (d->m_selectedWidget == w && d->m_selectedPart == part) {
#ifdef DEBUG_PARTMANAGER
                    qDebug() << "Part " << part << " made active (from selected) because " << w->metaObject()->className() << " got event" << " " << evType;
#endif
                    d->setReason(ev);
                    setActivePart(part, w);
                    d->m_reason = NoReason;
                    return true;
                } else if (d->m_activeWidget == w && d->m_activePart == part) {
                    setSelectedPart(nullptr);
                    return false;
                }

                return false;
            } else if (part != d->m_activePart && d->allowExplicitFocusEvent(ev)) {
#ifdef DEBUG_PARTMANAGER
                qDebug() << "Part " << part << " made active because " << w->metaObject()->className() << " got event" << " " << evType;
#endif
                d->setReason(ev);
                setActivePart(part, w);
                d->m_reason = NoReason;
            }

            return false;
        }
#ifdef QT_WIDGETS_LIB
        w = w->parentWidget();

        if (w && (((w->windowFlags() & Qt::Dialog) && w->isModal()) ||
                  (w->windowFlags() & Qt::Popup) || (w->windowFlags() & Qt::Tool))) {
#ifdef DEBUG_PARTMANAGER
            qDebug() << QString("No part made active although %1/%2 got event - loop aborted").arg(obj->objectName()).arg(obj->metaObject()->className());
#endif
            return false;
        }
#else
        w = w->parentItem();
#endif
    }

#ifdef DEBUG_PARTMANAGER
    qDebug() << QString("No part made active although %1/%2 got event").arg(obj->objectName()).arg(obj->metaObject()->className());
#endif
    return false;
}

Part *PartManager::findPartFromWidget(QWidget *widget, const QPoint &pos)
{
    for (QList<Part *>::iterator it = d->m_parts.begin(), end = d->m_parts.end(); it != end; ++it) {
        Part *part = (*it)->hitTest(widget, pos);
        if (part && d->m_parts.contains(part)) {
            return part;
        }
    }
    return nullptr;
}

Part *PartManager::findPartFromWidget(QWidget *widget)
{
    for (QList<Part *>::iterator it = d->m_parts.begin(), end = d->m_parts.end(); it != end; ++it) {
        if (widget == (*it)->widget()) {
            return (*it);
        }
    }
    return nullptr;
}

void PartManager::addPart(Part *part, bool setActive)
{
    Q_ASSERT(part);

    // don't add parts more than once :)
    if (d->m_parts.contains(part)) {
#ifdef DEBUG_PARTMANAGER
        qWarning() << part << " already added";
#endif
        return;
    }

    d->m_parts.append(part);

    part->setManager(this);

    if (setActive) {
        setActivePart(part);

        if (QWidget *w = part->widget()) {
#ifdef QT_WIDGETS_LIB
            // Prevent focus problems
            if (w->focusPolicy() == Qt::NoFocus) {
                qWarning() << "Part '" << part->objectName() << "' has a widget "
                           << w->objectName() << " with a focus policy of NoFocus. It should have at least a"
                           << "ClickFocus policy, for part activation to work well." << endl;
            }
            if (part->widget() && part->widget()->focusPolicy() == Qt::TabFocus) {
                qWarning() << "Part '" << part->objectName() << "' has a widget "
                           << w->objectName() << " with a focus policy of TabFocus. It should have at least a"
                           << "ClickFocus policy, for part activation to work well." << endl;
            }
            w->setFocus();
            w->show();
#endif
        }
    }
    emit partAdded(part);
}

void PartManager::removePart(Part *part)
{
    if (!d->m_parts.contains(part)) {
        return;
    }

    const int nb = d->m_parts.removeAll(part);
    Q_ASSERT(nb == 1);
    Q_UNUSED(nb); // no warning in release mode
    part->setManager(nullptr);

    emit partRemoved(part);

    if (part == d->m_activePart) {
        setActivePart(nullptr);
    }
    if (part == d->m_selectedPart) {
        setSelectedPart(nullptr);
    }
}

void PartManager::replacePart(Part *oldPart, Part *newPart, bool setActive)
{
    //qDebug() << "replacePart " << oldPart->name() << "-> " << newPart->name() << " setActive=" << setActive;
    // This methods does exactly removePart + addPart but without calling setActivePart(0) in between
    if (!d->m_parts.contains(oldPart)) {
        qFatal("Can't remove part %s, not in KPartManager's list.", oldPart->objectName().toLocal8Bit().constData());
        return;
    }

    d->m_parts.removeAll(oldPart);
    oldPart->setManager(nullptr);

    emit partRemoved(oldPart);

    addPart(newPart, setActive);
}

void PartManager::setActivePart(Part *part, QWidget *widget)
{
    if (part && !d->m_parts.contains(part)) {
        qWarning() << "trying to activate a non-registered part!" << part->objectName();
        return; // don't allow someone call setActivePart with a part we don't know about
    }

    //check whether nested parts are disallowed and activate the top parent part then, by traversing the
    //tree recursively (Simon)
    if (part && !d->m_bAllowNestedParts) {
        QObject *parentPart = part->parent(); // ### this relies on people using KParts::Factory!
        KParts::Part *parPart = ::qobject_cast<KParts::Part *>(parentPart);
        if (parPart) {
            setActivePart(parPart, parPart->widget());
            return;
        }
    }

#ifdef DEBUG_PARTMANAGER
    qDebug() << "PartManager::setActivePart d->m_activePart=" << d->m_activePart << "<->part=" << part
             << " d->m_activeWidget=" << d->m_activeWidget << "<->widget=" << widget << endl;
#endif

    // don't activate twice
    if (d->m_activePart && part && d->m_activePart == part &&
            (!widget || d->m_activeWidget == widget)) {
        return;
    }

    KParts::Part *oldActivePart = d->m_activePart;
    QWidget *oldActiveWidget = d->m_activeWidget;

    setSelectedPart(nullptr);

    d->m_activePart = part;
    d->m_activeWidget = widget;

    if (oldActivePart) {
        KParts::Part *savedActivePart = part;
        QWidget *savedActiveWidget = widget;
#ifdef PART_USE_ENEVT
        PartActivateEvent ev(false, oldActivePart, oldActiveWidget);
        QCoreApplication::sendEvent(oldActivePart, &ev);
        if (oldActiveWidget) {
            disconnect(oldActiveWidget, &QWidget::destroyed,
                       this, &PartManager::slotWidgetDestroyed);
            QCoreApplication::sendEvent(oldActiveWidget, &ev);
        }
#endif
        d->m_activePart = savedActivePart;
        d->m_activeWidget = savedActiveWidget;
    }

    if (d->m_activePart) {
        if (!widget) {
            d->m_activeWidget = part->widget();
        }
#ifdef PART_USE_ENEVT
        PartActivateEvent ev(true, d->m_activePart, d->m_activeWidget);
        QCoreApplication::sendEvent(d->m_activePart, &ev);
        if (d->m_activeWidget) {
            connect(d->m_activeWidget, &QWidget::destroyed,
                    this, &PartManager::slotWidgetDestroyed);
            QCoreApplication::sendEvent(d->m_activeWidget, &ev);
        }
#endif
    }
    // Set the new active instance
    //setActiveComponent(d->m_activePart ? d->m_activePart->componentData() : KComponentData::mainComponent());

#ifdef DEBUG_PARTMANAGER
    qDebug() << this << " emitting activePartChanged " << d->m_activePart;
#endif
    emit activePartChanged(d->m_activePart);
}

Part *PartManager::activePart() const
{
    return d->m_activePart;
}

QWidget *PartManager::activeWidget() const
{
    return  d->m_activeWidget;
}

void PartManager::setSelectedPart(Part *part, QWidget *widget)
{
    if (part == d->m_selectedPart && widget == d->m_selectedWidget) {
        return;
    }

    Part *oldPart = d->m_selectedPart;
    QWidget *oldWidget = d->m_selectedWidget;

    d->m_selectedPart = part;
    d->m_selectedWidget = widget;

    if (part && !widget) {
        d->m_selectedWidget = part->widget();
    }

    if (oldPart) {
#ifdef PART_USE_ENEVT
        PartSelectEvent ev(false, oldPart, oldWidget);
        QCoreApplication::sendEvent(oldPart, &ev);
        QCoreApplication::sendEvent(oldWidget, &ev);
#endif
    }

    if (d->m_selectedPart) {
#ifdef PART_USE_ENEVT
        PartSelectEvent ev(true, d->m_selectedPart, d->m_selectedWidget);
        QCoreApplication::sendEvent(d->m_selectedPart, &ev);
        QCoreApplication::sendEvent(d->m_selectedWidget, &ev);
#endif
    }
}

Part *PartManager::selectedPart() const
{
    return d->m_selectedPart;
}

QWidget *PartManager::selectedWidget() const
{
    return d->m_selectedWidget;
}

void PartManager::slotObjectDestroyed()
{
    // qDebug();
    removePart(const_cast<Part *>(static_cast<const Part *>(sender())));
}

void PartManager::slotWidgetDestroyed()
{
    // qDebug();
    if (static_cast<const QWidget *>(sender()) == d->m_activeWidget) {
        setActivePart(nullptr);    //do not remove the part because if the part's widget dies, then the
    }
    //part will delete itself anyway, invoking removePart() in its destructor
}

const QList<Part *> PartManager::parts() const
{
    return d->m_parts;
}

void PartManager::addManagedTopLevelWidget(const QWidget *topLevel)
{
#ifdef QT_WIDGETS_LIB
    if (!topLevel->isTopLevel()) {
        return;
    }
#endif

    if (d->m_managedTopLevelWidgets.contains(topLevel)) {
        return;
    }

    d->m_managedTopLevelWidgets.append(topLevel);
    connect(topLevel, &QWidget::destroyed,
            this, &PartManager::slotManagedTopLevelWidgetDestroyed);
}

void PartManager::removeManagedTopLevelWidget(const QWidget *topLevel)
{
    d->m_managedTopLevelWidgets.removeAll(topLevel);
}

void PartManager::slotManagedTopLevelWidgetDestroyed()
{
    const QWidget *widget = static_cast<const QWidget *>(sender());
    removeManagedTopLevelWidget(widget);
}

int PartManager::reason() const
{
    return d->m_reason;
}

void PartManager::setIgnoreExplictFocusRequests(bool ignore)
{
    d->m_bIgnoreExplicitFocusRequest = ignore;
}
