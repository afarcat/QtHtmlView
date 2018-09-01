/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999-2005 David Faure <faure@kde.org>
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

#ifndef _KPARTS_PART_P_H
#define _KPARTS_PART_P_H

#include "part.h"

#ifdef QT_WIDGETS_LIB
#include <QWidget>
#else
#include <QQuickItem>
#define QWidget QQuickItem
#endif
#include <QPointer>


namespace KParts
{

class PartPrivate
{
public:
    Q_DECLARE_PUBLIC(Part)

    PartPrivate(Part *q)
        : q_ptr(q),
          m_obj(nullptr),
          m_bSelectable(true),
          m_autoDeleteWidget(true),
          m_autoDeletePart(true),
          m_manager(nullptr)
    {
    }

    ~PartPrivate()
    {
    }

    Part *q_ptr;
    QObject *m_obj;
    bool m_bSelectable;
    bool m_autoDeleteWidget;
    bool m_autoDeletePart;
    PartManager *m_manager;
    QPointer<QWidget> m_widget;
};

} // namespace

#endif
