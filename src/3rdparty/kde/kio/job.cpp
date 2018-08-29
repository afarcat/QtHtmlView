/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                  2000-2009 David Faure <faure@kde.org>
                       Waldo Bastian <bastian@kde.org>

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

#include "job.h"
#include "job_p.h"

#include <time.h>

#include <QFile>
#include <QLinkedList>
#include <QDateTime>
#include <QTimer>
#include <QCoreApplication>

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QSsl>
#include <QSslSocket>
#include <QSslConfiguration>

QNetworkAccessManager *g_nam = nullptr;

using namespace KIO;

Job::Job() : KJob(qApp)
    , d_ptr(new JobPrivate)
{
    d_ptr->q_ptr = this;
    setCapabilities(KJob::Killable | KJob::Suspendable);

    QObject::connect(&d_ptr->m_timerCheck, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
    QObject::connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

Job::Job(JobPrivate &dd) : KJob(qApp)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    setCapabilities(KJob::Killable | KJob::Suspendable);

    QObject::connect(&d_ptr->m_timerCheck, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
    QObject::connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

Job::~Job()
{
#ifdef QT_DEBUG
    qDebug() << "Job::~Job...url=" << this;
#endif

    doKill();

    delete d_ptr;
}

void Job::init()
{
    if (g_nam == nullptr) {
        g_nam = new QNetworkAccessManager(qApp);
    }
}

void Job::clear()
{
    if (g_nam != nullptr) {
        g_nam->deleteLater();
        g_nam = nullptr;
    }
}

#ifdef KIO_SUB_JOB
bool Job::addSubjob(KJob *jobBase)
{
    //qDebug() << "addSubjob(" << jobBase << ") this=" << this;

    bool ok = KJob::addSubjob(jobBase);
    KIO::Job *job = dynamic_cast<KIO::Job *>(jobBase);
    if (ok && job) {
        // Copy metadata into subjob (e.g. window-id, user-timestamp etc.)
        Q_D(Job);
        job->mergeMetaData(d->m_outgoingMetaData);

        // Forward information from that subjob.
        connect(job, SIGNAL(speed(KJob*,ulong)),
                SLOT(slotSpeed(KJob*,ulong)));

        job->setProperty("window", property("window")); // see KJobWidgets
        job->setProperty("userTimestamp", property("userTimestamp")); // see KJobWidgets
    }
    return ok;
}

bool Job::removeSubjob(KJob *jobBase)
{
    //qDebug() << "removeSubjob(" << jobBase << ") this=" << this << "subjobs=" << subjobs().count();
    return KJob::removeSubjob(jobBase);
}
#endif

void Job::slotStart()
{
    Q_D(Job);

    doKill();

    setError(KJob::NoError);

    if (d->m_command == CMD_GET) {
        if (d->m_url.isLocalFile()) {
            QFile file(d->m_url.toLocalFile());

            if (file.open(QIODevice::ReadOnly)) {
                QByteArray dat = file.readAll();
                file.close();

                emit data(this, dat);
            }
            else {
                qDebug() << "cannot open for reading...file =" << file.fileName();
                setError(KIO::ERR_CANNOT_OPEN_FOR_READING);
            }

            emitResult();
            return;
        }
    }

    //Data URI scheme: RFC2397
    if (d->m_url.scheme() == "data") {
        QByteArray dat = d->m_url.toString().toUtf8();
        if (dat.startsWith("data:image/png;base64,")) {
            dat = QByteArray::fromBase64(dat.mid(qstrlen("data:image/png;base64,")));

            emit data(this, dat);
            emitResult();
            return;
        }
    }

    if (g_nam == nullptr) {
        setError(KIO::ERR_INTERNAL);
        emitResult();
        return;
    }

#ifdef QT_DEBUG
    qDebug() << "Job::slotStart...url=" << this << d->m_url;
#endif

    QNetworkRequest request;

    if (d->m_url.scheme() == "https") {
#ifndef QT_NO_SSL
        QSslConfiguration config = request.sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1SslV3);
        request.setSslConfiguration(config);
#endif
    }

    //HTTP ACTION
    if (d->m_command == CMD_GET) {
        d->m_reply = g_nam->get(QNetworkRequest(d->m_url));
    }
    //else if (d->m_command == CMD_PUT) {
    //    d->m_reply = g_nam->put(QNetworkRequest(d->m_url));
    //}
    //else if (d->m_command == CMD_POST) {
    //    d->m_reply = g_nam->post(QNetworkRequest(d->m_url));
    //}

    if (!d->m_reply){
        setError(KIO::ERR_INTERNAL);
        emitResult();
        return;
    }

    d->m_reply->ignoreSslErrors();

    QObject::connect(d->m_reply, SIGNAL(metaDataChanged()), this, SLOT(slotMetaDataChanged()));
    QObject::connect(d->m_reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    QObject::connect(d->m_reply, SIGNAL(finished()), this, SLOT(slotFinished()));
    QObject::connect(d->m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
#ifndef QT_NO_OPENSSL
    QObject::connect(d->m_reply, SIGNAL(sslErrors(const QList<QSslError>&)), this, SLOT(slotSslErrors(const QList<QSslError>&)));
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QObject::connect(d->m_reply, SIGNAL(redirected(const QUrl &)), this, SLOT(slotRedirected(const QUrl &)));
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    QObject::connect(d->m_reply, SIGNAL(redirectAllowed()), this, SLOT(slotRedirectAllowed()));
#endif
    QObject::connect(d->m_reply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(slotUploadProgress(qint64, qint64)));
    QObject::connect(d->m_reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(slotDownloadProgress(qint64, qint64)));

    d->m_timerCheck.stop();
    d->m_timerCheck.start(d->timeoutValue);
}

void Job::slotError(int, const QString &)
{

}

void Job::slotMetaDataChanged()
{

}

void Job::slotReadyRead()
{
    Q_D(Job);

    QByteArray dat = d->m_reply->readAll();

    emit data(this, dat);
}

void Job::slotFinished()
{
    doKill();

    emitResult();
}

void Job::slotError(QNetworkReply::NetworkError err)
{
    setError(err);
}

#ifndef QT_NO_OPENSSL
void Job::slotSslErrors(const QList<QSslError> &errors)
{

}
#endif

void Job::slotRedirected(const QUrl &url)
{
    qDebug()<<"Job::slotRedirected...url="<<url;
}

void Job::slotRedirectAllowed()
{

}
void Job::slotUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    Q_D(Job);

    d->fileSentSize = bytesSent;
}

void Job::slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_D(Job);

    d->fileRecvSize = bytesReceived;
}

void Job::slotTimeOut()
{
    Q_D(Job);

    if (d->m_command == CMD_GET) {
        if (d->lastRecvSize != d->fileRecvSize) {
            d->lastRecvSize = d->fileRecvSize;
        } else {
            doKill();

            setError(KIO::ERR_ABORTED);
            emitResult();
        }
    }
}

bool Job::doKill()
{
#ifdef KIO_SUB_JOB
    // kill all subjobs, without triggering their result slot
    Q_FOREACH (KJob *it, subjobs()) {
        it->kill(KJob::Quietly);
    }
    clearSubjobs();
#endif

    Q_D(Job);

    d->clear();
    d->stopTimerCheck();

    return true;
}

bool Job::doSuspend()
{
#ifdef KIO_SUB_JOB
    Q_FOREACH (KJob *it, subjobs()) {
        if (!it->suspend()) {
            return false;
        }
    }
#endif

    return true;
}

bool Job::doResume()
{
#ifdef KIO_SUB_JOB
    Q_FOREACH (KJob *it, subjobs()) {
        if (!it->resume()) {
            return false;
        }
    }
#endif

    return true;
}

bool Job::isErrorPage() const
{
    return false;
}

void Job::setParentJob(Job *job)
{
    Q_D(Job);
    Q_ASSERT(d->m_parentJob == nullptr);
    Q_ASSERT(job);
    d->m_parentJob = job;
}

Job *Job::parentJob() const
{
    return d_func()->m_parentJob;
}

MetaData Job::metaData() const
{
    return d_func()->m_incomingMetaData;
}

QString Job::queryMetaData(const QString &key)
{
    return d_func()->m_incomingMetaData.value(key, QString());
}

void Job::setMetaData(const KIO::MetaData &_metaData)
{
    Q_D(Job);
    d->m_outgoingMetaData = _metaData;
}

void Job::addMetaData(const QString &key, const QString &value)
{
    d_func()->m_outgoingMetaData.insert(key, value);
}

void Job::addMetaData(const QMap<QString, QString> &values)
{
    Q_D(Job);
    QMap<QString, QString>::const_iterator it = values.begin();
    for (; it != values.end(); ++it) {
        d->m_outgoingMetaData.insert(it.key(), it.value());
    }
}

void Job::mergeMetaData(const QMap<QString, QString> &values)
{
    Q_D(Job);
    QMap<QString, QString>::const_iterator it = values.begin();
    for (; it != values.end(); ++it)
        // there's probably a faster way
        if (!d->m_outgoingMetaData.contains(it.key())) {
            d->m_outgoingMetaData.insert(it.key(), it.value());
        }
}

MetaData Job::outgoingMetaData() const
{
    return d_func()->m_outgoingMetaData;
}

QString Job::errorString() const
{
#ifdef KIO_FULL
#else
    return QString();
#endif
}

QStringList Job::detailedErrorStrings(const QUrl *reqUrl /*= 0*/,
        int method /*= -1*/) const
{
#ifdef KIO_FULL
#else
    return QStringList();
#endif
}

void JobPrivate::jobInit()
{
    Q_Q(Job);

    if (!m_url.isValid() || m_url.scheme().isEmpty()) {
        qCWarning(KIO_CORE) << "Invalid URL:" << m_url;
        q->setError(ERR_MALFORMED_URL);
        q->setErrorText(m_url.toString());
        QTimer::singleShot(0, q, SLOT(slotFinished()));
        return;
    }

    QTimer::singleShot(0, q, SLOT(slotStart()));
}

void JobPrivate::stopTimerCheck()
{
    fileSentSize = 0;
    lastSentSize = 0;

    fileRecvSize = 0;
    lastRecvSize = 0;

    m_timerCheck.stop();
}

void JobPrivate::slotSpeed(KJob *, unsigned long speed)
{
    //qDebug() << speed;
    q_func()->emitSpeed(speed);
}

//API FUNCTIONS
Job *KIO::get(const QUrl &url, LoadType reload, JobFlags flags)
{
    // Send decoded path and encoded query
    KIO_ARGS << url;
    Job *job = JobPrivate::newJob(url, CMD_GET, packedArgs, QByteArray(), flags);
    if (reload == Reload) {
        job->addMetaData(QStringLiteral("cache"), QStringLiteral("reload"));
    }
    return job;
}

Job *KIO::put(const QUrl &url, int permissions, JobFlags flags)
{
    KIO_ARGS << url << qint8((flags & Overwrite) ? 1 : 0) << qint8((flags & Resume) ? 1 : 0) << permissions;
    return JobPrivate::newJob(url, CMD_PUT, packedArgs, QByteArray(), flags);
}

Job *KIO::http_post(const QUrl &url, const QByteArray &postData, JobFlags flags)
{
    //AFA: Function not implemented
//    bool redirection = false;
//    QUrl _url(url);
//    if (_url.path().isEmpty()) {
//        redirection = true;
//        _url.setPath(QStringLiteral("/"));
//    }

//    Job *job = precheckHttpPost(_url, postData, flags);
//    if (job) {
//        return job;
//    }

//    // Send http post command (1), decoded path and encoded query
//    KIO_ARGS << (int)1 << _url << static_cast<qint64>(postData.size());
//    job = JobPrivate::newJob(_url, CMD_SPECIAL, packedArgs, postData, flags);

//    if (redirection) {
//        QTimer::singleShot(0, job, SLOT(slotPostRedirection()));
//    }

//    return job;
    return nullptr;
}

Job *KIO::http_post(const QUrl &url, QIODevice *device, qint64 size, JobFlags flags)
{
    //AFA: Function not implemented
//    bool redirection = false;
//    QUrl _url(url);
//    if (_url.path().isEmpty()) {
//        redirection = true;
//        _url.setPath(QStringLiteral("/"));
//    }

//    Job *job = precheckHttpPost(_url, ioDevice, flags);
//    if (job) {
//        return job;
//    }

//    // If no size is specified and the QIODevice is not a sequential one,
//    // attempt to obtain the size information from it.
//    Q_ASSERT(ioDevice);
//    if (size < 0) {
//        size = ((ioDevice && !ioDevice->isSequential()) ? ioDevice->size() : -1);
//    }

//    // Send http post command (1), decoded path and encoded query
//    KIO_ARGS << (int)1 << _url << size;
//    job = JobPrivate::newJob(_url, CMD_SPECIAL, packedArgs, ioDevice, flags);

//    if (redirection) {
//        QTimer::singleShot(0, job, SLOT(slotPostRedirection()));
//    }

//    return job;
    return nullptr;
}

Job *KIO::http_delete(const QUrl &url, JobFlags flags)
{
    //AFA: Function not implemented
//    // Send decoded path and encoded query
//    KIO_ARGS << url;
//    Job *job = JobPrivate::newJob(url, CMD_DEL, packedArgs, QByteArray(), flags);
//    return job;
    return nullptr;
}

Job *KIO::http_update_cache(const QUrl &url, bool no_cache, const QDateTime &expireDate)
{
    //AFA: Function not implemented
//    Q_ASSERT(url.scheme() == QLatin1String("http") || url.scheme() == QLatin1String("https"));
//    // Send http update_cache command (2)
//    KIO_ARGS << (int)2 << url << no_cache << qlonglong(expireDate.toMSecsSinceEpoch() / 1000);
//    Job *job = JobPrivate::newJob(url, CMD_SPECIAL, packedArgs, QByteArray());
//    return job;
    return nullptr;
}

#include "moc_job.cpp"
