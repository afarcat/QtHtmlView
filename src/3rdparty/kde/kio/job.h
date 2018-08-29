/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                  2000-2009 David Faure <faure@kde.org>

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

#ifndef KIO_JOB_BASE_H
#define KIO_JOB_BASE_H

#include "global.h"

#include "kjob.h"
#include <kio/metadata.h>

#include <QNetworkReply>

namespace KIO
{

class JobPrivate;
/**
 * @class KIO::Job job_base.h <KIO/Job>
 *
 * The base class for all jobs.
 * For all jobs created in an application, the code looks like
 *
 * \code
 *   KIO::Job * job = KIO::someoperation( some parameters );
 *   connect( job, SIGNAL( result( KJob * ) ),
 *            this, SLOT( slotResult( KJob * ) ) );
 * \endcode
 *   (other connects, specific to the job)
 *
 * And slotResult is usually at least:
 *
 * \code
 *  if ( job->error() )
 *      job->uiDelegate()->showErrorMessage();
 * \endcode
 * @see KIO::Scheduler
 */
class KIOCORE_EXPORT Job : public KJob
{
    Q_OBJECT

protected:
    Job();
    Job(JobPrivate &dd);

public:
    virtual ~Job();
    void start() override {} // Since KIO autostarts its jobs

public:
    static void init();
    static void clear();

protected:
    /**
     * Abort this job.
     * This kills all subjobs and deletes the job.
     *
     */
    bool doKill() override;

    /**
     * Suspend this job
     * @see resume
     */
    bool doSuspend() override;

    /**
     * Resume this job
     * @see suspend
     */
    bool doResume() override;

public:
    /**
     * Checks whether we got an error page. This currently only happens
     * with HTTP urls. Call this from your slot connected to result().
     *
     * @return true if we got an (HTML) error page from the server
     * instead of what we asked for.
     */
    bool isErrorPage() const;

    /**
     * Converts an error code and a non-i18n error message into an
     * error message in the current language. The low level (non-i18n)
     * error message (usually a url) is put into the translated error
     * message using %1.
     *
     * Example for errid == ERR_CANNOT_OPEN_FOR_READING:
     * \code
     *   i18n( "Could not read\n%1" ).arg( errortext );
     * \endcode
     * Use this to display the error yourself, but for a dialog box
     * use uiDelegate()->showErrorMessage(). Do not call it if error()
     * is not 0.
     * @return the error message and if there is no error, a message
     *         telling the user that the app is broken, so check with
     *         error() whether there is an error
     */
    QString errorString() const override;

    /**
     * Converts an error code and a non-i18n error message into i18n
     * strings suitable for presentation in a detailed error message box.
     *
     * @param reqUrl the request URL that generated this error message
     * @param method the method that generated this error message
     * (unimplemented)
     * @return the following strings: caption, error + description,
     *         causes+solutions
     */
    QStringList detailedErrorStrings(const QUrl *reqUrl = nullptr,
                                     int method = -1) const;

    /**
     * Set the parent Job.
     * One example use of this is when FileCopyJob calls RenameDialog::open,
     * it must pass the correct progress ID of the parent CopyJob
     * (to hide the progress dialog).
     * You can set the parent job only once. By default a job does not
     * have a parent job.
     * @param parentJob the new parent job
     */
    void setParentJob(Job *parentJob);

    /**
     * Returns the parent job, if there is one.
     * @return the parent job, or @c nullptr if there is none
     * @see setParentJob
     */
    Job *parentJob() const;

    /**
     * Set meta data to be sent to the slave, replacing existing
     * meta data.
     * @param metaData the meta data to set
     * @see addMetaData()
     * @see mergeMetaData()
     */
    void setMetaData(const KIO::MetaData &metaData);

    /**
     * Add key/value pair to the meta data that is sent to the slave.
     * @param key the key of the meta data
     * @param value the value of the meta data
     * @see setMetaData()
     * @see mergeMetaData()
     */
    void addMetaData(const QString &key, const QString &value);

    /**
     * Add key/value pairs to the meta data that is sent to the slave.
     * If a certain key already existed, it will be overridden.
     * @param values the meta data to add
     * @see setMetaData()
     * @see mergeMetaData()
     */
    void addMetaData(const QMap<QString, QString> &values);

    /**
     * Add key/value pairs to the meta data that is sent to the slave.
     * If a certain key already existed, it will remain unchanged.
     * @param values the meta data to merge
     * @see setMetaData()
     * @see addMetaData()
     */
    void mergeMetaData(const QMap<QString, QString> &values);

    /**
     * @internal. For the scheduler. Do not use.
     */
    MetaData outgoingMetaData() const;

    /**
     * Get meta data received from the slave.
     * (Valid when first data is received and/or slave is finished)
     * @return the job's meta data
     */
    MetaData metaData() const;

    /**
     * Query meta data received from the slave.
     * (Valid when first data is received and/or slave is finished)
     * @param key the key of the meta data to retrieve
     * @return the value of the meta data, or QString() if the
     *         @p key does not exist
     */
    QString queryMetaData(const QString &key);

protected:

Q_SIGNALS:
    /**
     * Emitted when the slave successfully connected to the host.
     * There is no guarantee the slave will send this, and this is
     * currently unused (in the applications).
     * @param job the job that emitted this signal
     */
    void connected(KIO::Job *job);

    /**
     * Data from the slave has arrived.
     * @param job the job that emitted this signal
     * @param data data received from the slave.
     *
     * End of data (EOD) has been reached if data.size() == 0, however, you
     * should not be certain of data.size() == 0 ever happening (e.g. in case
     * of an error), so you should rely on result() instead.
     */
    void data(KIO::Job *job, const QByteArray &data);

    /**
     * Request for data.
     * Please note, that you shouldn't put too large chunks
     * of data in it as this requires copies within the frame
     * work, so you should rather split the data you want
     * to pass here in reasonable chunks (about 1MB maximum)
     *
     * @param job the job that emitted this signal
     * @param data buffer to fill with data to send to the
     * slave. An empty buffer indicates end of data. (EOD)
     */
    void dataReq(KIO::Job *job, QByteArray &data);

    /**
     * Signals a redirection.
     * Use to update the URL shown to the user.
     * The redirection itself is handled internally.
     * @param job the job that emitted this signal
     * @param url the new URL
     */
    void redirection(KIO::Job *job, const QUrl &url);

    /**
     * Signals a permanent redirection.
     * The redirection itself is handled internally.
     * @param job the job that emitted this signal
     * @param fromUrl the original URL
     * @param toUrl the new URL
     */
    void permanentRedirection(KIO::Job *job, const QUrl &fromUrl, const QUrl &toUrl);

    /**
     * Mimetype determined.
     * @param job the job that emitted this signal
     * @param type the mime type
     */
    void mimetype(KIO::Job *job, const QString &type);

    /**
     * @internal
     * Emitted if the "put" job found an existing partial file
     * (in which case offset is the size of that file)
     * and emitted by the "get" job if it supports resuming to
     * the given offset - in this case @p offset is unused)
     */
    void canResume(KIO::Job *job, KIO::filesize_t offset);

#ifdef KIO_SUB_JOB
protected:
    /**
     * Add a job that has to be finished before a result
     * is emitted. This has obviously to be called before
     * the finish signal is emitted by the slave.
     *
     * @param job the subjob to add
     */
    bool addSubjob(KJob *job) override;

    /**
     * Mark a sub job as being done.
     *
     * Note that this does not terminate the parent job, even if @p job
     * is the last subjob.  emitResult must be called to indicate that
     * the job is complete.
     *
     * @param job the subjob to remove
     */
    bool removeSubjob(KJob *job) override;
#endif

protected slots:
    void slotStart();

    void slotError(int, const QString &);

private slots:
    void slotMetaDataChanged();
    void slotReadyRead();
    void slotFinished();
    void slotError(QNetworkReply::NetworkError);
#ifndef QT_NO_OPENSSL
    void slotSslErrors( const QList<QSslError> & errors);
#endif
    void slotRedirected(const QUrl &url);
    void slotRedirectAllowed();
    void slotUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    void slotTimeOut();

protected:
    JobPrivate *const d_ptr;

private:
    /**
     * Forward signal from subjob.
     * @param job the subjob
     * @param speed the speed in bytes/s
     * @see speed()
     */
    Q_PRIVATE_SLOT(d_func(), void slotSpeed(KJob *job, unsigned long speed))
    Q_DECLARE_PRIVATE(Job)
};

/**
 * Flags for the job properties.
 * Not all flags are supported in all cases. Please see documentation of
 * the calling function!
 */
enum JobFlag {
    /**
     * Show the progress info GUI, no Resume and no Overwrite
     */
    DefaultFlags = 0,

    /**
     * Hide progress information dialog, i.e. don't show a GUI.
     */
    HideProgressInfo = 1,

    /**
     * When set, automatically append to the destination file if it exists already.
     * WARNING: this is NOT the builtin support for offering the user to resume a previous
     * partial download. The Resume option is much less used, it allows to append
     * to an existing file.
     * This is used by KIO::put(), KIO::file_copy(), KIO::file_move().
     */
    Resume = 2,

    /**
     * When set, automatically overwrite the destination if it exists already.
     * This is used by KIO::rename(), KIO::put(), KIO::file_copy(), KIO::file_move(), KIO::symlink().
     * Otherwise the operation will fail with ERR_FILE_ALREADY_EXIST or ERR_DIR_ALREADY_EXIST.
     */
    Overwrite = 4,

    /**
     * When set, notifies the slave that application/job does not want privilege execution.
     * So in case of failure due to insufficient privileges show an error without attempting
     * to run the operation as root first.
     *
     * @since 5.43
     */
    NoPrivilegeExecution = 8,
};
Q_DECLARE_FLAGS(JobFlags, JobFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(JobFlags)

enum LoadType { Reload, NoReload };

//API FUNCTIONS
/**
 * Get (means: read).
 * This is the job to use in order to "download" a file into memory.
 * The slave emits the data through the data() signal.
 *
 * Special case: if you want to determine the mimetype of the file first,
 * and then read it with the appropriate component, you can still use
 * a KIO::get() directly. When that job emits the mimeType signal, (which is
 * guaranteed to happen before it emits any data), put the job on hold:
 *
 * @code
 *   job->putOnHold();
 *   KIO::Scheduler::publishSlaveOnHold();
 * @endcode
 *
 * and forget about the job. The next time someone does a KIO::get() on the
 * same URL (even in another process) this job will be resumed. This saves KIO
 * from doing two requests to the server.
 *
 * @param url the URL of the file
 * @param reload Reload to reload the file, NoReload if it can be taken from the cache
 * @param flags Can be HideProgressInfo here
 * @return the job handling the operation.
 */
KIOCORE_EXPORT Job *get(const QUrl &url, LoadType reload = NoReload, JobFlags flags = DefaultFlags);

/**
 * Put (means: write)
 *
 * @param url Where to write data.
 * @param permissions May be -1. In this case no special permission mode is set.
 * @param flags Can be HideProgressInfo, Overwrite and Resume here. WARNING:
 * Setting Resume means that the data will be appended to @p dest if @p dest exists.
 * @return the job handling the operation.
 * @see multi_get()
 */
KIOCORE_EXPORT Job *put(const QUrl &url, int permissions,
                                JobFlags flags = DefaultFlags);

/**
 * HTTP POST (for form data).
 *
 * Example:
 * \code
 *    job = KIO::http_post( url, postData, KIO::HideProgressInfo );
 *    job->addMetaData("content-type", contentType );
 *    job->addMetaData("referrer", referrerURL);
 * \endcode
 *
 * @p postData is the data that you want to send and
 * @p contentType is the complete HTTP header line that
 * specifies the content's MIME type, for example
 * "Content-Type: text/xml".
 *
 * You MUST specify content-type!
 *
 * Often @p contentType is
 * "Content-Type: application/x-www-form-urlencoded" and
 * the @p postData is then an ASCII string (without null-termination!)
 * with characters like space, linefeed and percent escaped like %20,
 * %0A and %25.
 *
 * @param url Where to write the data.
 * @param postData Encoded data to post.
 * @param flags Can be HideProgressInfo here
 * @return the job handling the operation.
 */
KIOCORE_EXPORT Job *http_post(const QUrl &url, const QByteArray &postData,
                                      JobFlags flags = DefaultFlags);

/**
 * HTTP POST.
 *
 * This function, unlike the one that accepts a QByteArray, accepts an IO device
 * from which to read the encoded data to be posted to the server in order to
 * to avoid holding the content of very large post requests, e.g. multimedia file
 * uploads, in memory.
 *
 * @param url Where to write the data.
 * @param device the device to read from
 * @param size Size of the encoded post data.
 * @param flags Can be HideProgressInfo here
 * @return the job handling the operation.
 *
 * @since 4.7
 */
KIOCORE_EXPORT Job *http_post(const QUrl &url, QIODevice *device,
                                      qint64 size = -1, JobFlags flags = DefaultFlags);

/**
 * HTTP DELETE.
 *
 * Though this function servers the same purpose as KIO::file_delete, unlike
 * file_delete it accommodates HTTP sepecific actions such as redirections.
 *
 * @param url url resource to delete.
 * @param flags Can be HideProgressInfo here
 * @return the job handling the operation.
 *
 * @since 4.7.3
 */
KIOCORE_EXPORT Job *http_delete(const QUrl &url, JobFlags flags = DefaultFlags);

/**
 * HTTP cache update
 *
 * @param url Url to update, protocol must be "http".
 * @param no_cache If true, cache entry for @p url is deleted.
 * @param expireDate Local machine time indicating when the entry is
 * supposed to expire.
 * @return the job handling the operation.
 */
KIOCORE_EXPORT Job *http_update_cache(const QUrl &url, bool no_cache, const QDateTime &expireDate);

}

#endif
