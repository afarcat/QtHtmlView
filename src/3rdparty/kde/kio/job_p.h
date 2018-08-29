/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                  2000-2009 David Faure <faure@kde.org>
                       Waldo Bastian <bastian@kde.org>
    Copyright (C) 2007 Thiago Macieira <thiago@kde.org>
    Copyright (C) 2013 Dawit Alemayehu <adawit@kde.org>

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

#ifndef KIO_JOB_P_H
#define KIO_JOB_P_H

#include "job.h"
#include "commands_p.h"
#include <kio/metadata.h>

#include <QUrl>
#include <QPointer>
#include <QDataStream>
#include <QTimer>

#include <QNetworkRequest>
#include <QNetworkReply>

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( &packedArgs, QIODevice::WriteOnly ); stream

namespace KIO
{
class KIOCORE_EXPORT JobPrivate
{
public:
    JobPrivate()
        : m_parentJob(nullptr), m_extraFlags(0),
          m_privilegeExecutionEnabled(false)
    {
        m_reply = nullptr;
    }

    virtual ~JobPrivate() {

    }

    void clear() {
        if (m_reply){
            m_reply->deleteLater();
            m_reply = 0;
        }
    }

    /**
     * Some extra storage space for jobs that don't have their own
     * private d pointer.
     */
    enum { EF_TransferJobAsync    = (1 << 0),
           EF_TransferJobNeedData = (1 << 1),
           EF_TransferJobDataSent = (1 << 2),
           EF_ListJobUnrestricted = (1 << 3),
           EF_KillCalled          = (1 << 4)
         };

    // Maybe we could use the QObject parent/child mechanism instead
    // (requires a new ctor, and moving the ctor code to some init()).
    Job *m_parentJob;
    int m_extraFlags;
    MetaData m_incomingMetaData;
    MetaData m_internalMetaData;
    MetaData m_outgoingMetaData;
    Job *q_ptr;
    // For privilege operation
    bool m_privilegeExecutionEnabled;
    QString m_caption, m_message;

    void slotSpeed(KJob *job, unsigned long speed);

    //
    QByteArray m_packedArgs;
    QUrl m_url;
    QUrl m_subUrl;
    int m_command;

    QNetworkReply *m_reply;
    int timeoutValue = 30000;
    qint64 fileSentSize = 0;
    qint64 lastSentSize = 0;
    qint64 fileRecvSize = 0;
    qint64 lastRecvSize = 0;
    QTimer m_timerCheck;

    void jobInit();
    void stopTimerCheck();

    Q_DECLARE_PUBLIC(Job)

    static inline Job *newJob(const QUrl &url, int command,
                              const QByteArray &packedArgs,
                              const QByteArray &_staticData,
                              JobFlags flags = HideProgressInfo)
    {
        Job *job = new Job(*new JobPrivate());

        job->d_func()->m_url = url;
        job->d_func()->m_command = command;
        job->d_func()->m_packedArgs = packedArgs;
        job->d_func()->m_url = url;

        job->d_func()->jobInit();

        return job;
    }
};

}

#endif
