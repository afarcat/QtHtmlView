#ifndef KIO_GLOBAL_H
#define KIO_GLOBAL_H

#include "kiocore_export.h"

#include <QString>

#include "kjob.h"

/**
 * @short A namespace for KIO globals
 *
 */
namespace KIO
{
/// 64-bit file offset
typedef qlonglong fileoffset_t;
/// 64-bit file size
typedef qulonglong filesize_t;

/**
 * Error codes that can be emitted by KIO.
 */
enum Error {
    ERR_CANNOT_OPEN_FOR_READING = KJob::UserDefinedError + 1,
    ERR_CANNOT_OPEN_FOR_WRITING = KJob::UserDefinedError + 2,
    ERR_CANNOT_LAUNCH_PROCESS = KJob::UserDefinedError + 3,
    ERR_INTERNAL = KJob::UserDefinedError + 4,
    ERR_MALFORMED_URL = KJob::UserDefinedError + 5,
    ERR_UNSUPPORTED_PROTOCOL = KJob::UserDefinedError + 6,
    ERR_NO_SOURCE_PROTOCOL = KJob::UserDefinedError + 7,
    ERR_UNSUPPORTED_ACTION = KJob::UserDefinedError + 8,
    ERR_IS_DIRECTORY = KJob::UserDefinedError + 9, ///< ... where a file was expected
    ERR_IS_FILE = KJob::UserDefinedError + 10, ///< ... where a directory was expected (e.g. listing)
    ERR_DOES_NOT_EXIST = KJob::UserDefinedError + 11,
    ERR_FILE_ALREADY_EXIST = KJob::UserDefinedError + 12,
    ERR_DIR_ALREADY_EXIST = KJob::UserDefinedError + 13,
    ERR_UNKNOWN_HOST = KJob::UserDefinedError + 14,
    ERR_ACCESS_DENIED = KJob::UserDefinedError + 15,
    ERR_WRITE_ACCESS_DENIED = KJob::UserDefinedError + 16,
    ERR_CANNOT_ENTER_DIRECTORY = KJob::UserDefinedError + 17,
    ERR_PROTOCOL_IS_NOT_A_FILESYSTEM = KJob::UserDefinedError + 18,
    ERR_CYCLIC_LINK = KJob::UserDefinedError + 19,
    ERR_USER_CANCELED = KJob::KilledJobError,
    ERR_CYCLIC_COPY = KJob::UserDefinedError + 21,
    ERR_COULD_NOT_CREATE_SOCKET = KJob::UserDefinedError + 22, ///< @deprecated
    ERR_CANNOT_CREATE_SOCKET = KJob::UserDefinedError + 22,
    ERR_COULD_NOT_CONNECT = KJob::UserDefinedError + 23, ///< @deprecated
    ERR_CANNOT_CONNECT = KJob::UserDefinedError + 23,
    ERR_CONNECTION_BROKEN = KJob::UserDefinedError + 24,
    ERR_NOT_FILTER_PROTOCOL = KJob::UserDefinedError + 25,
    ERR_COULD_NOT_MOUNT = KJob::UserDefinedError + 26, ///< @deprecated
    ERR_CANNOT_MOUNT = KJob::UserDefinedError + 26,
    ERR_COULD_NOT_UNMOUNT = KJob::UserDefinedError + 27, ///< @deprecated
    ERR_CANNOT_UNMOUNT = KJob::UserDefinedError + 27,
    ERR_COULD_NOT_READ = KJob::UserDefinedError + 28, ///< @deprecated
    ERR_CANNOT_READ = KJob::UserDefinedError + 28,
    ERR_COULD_NOT_WRITE = KJob::UserDefinedError + 29, ///< @deprecated
    ERR_CANNOT_WRITE = KJob::UserDefinedError + 29,
    ERR_COULD_NOT_BIND = KJob::UserDefinedError + 30, ///< @deprecated
    ERR_CANNOT_BIND = KJob::UserDefinedError + 30,
    ERR_COULD_NOT_LISTEN = KJob::UserDefinedError + 31, ///< @deprecated
    ERR_CANNOT_LISTEN = KJob::UserDefinedError + 31,
    ERR_COULD_NOT_ACCEPT = KJob::UserDefinedError + 32, ///< @deprecated
    ERR_CANNOT_ACCEPT = KJob::UserDefinedError + 32,
    ERR_COULD_NOT_LOGIN = KJob::UserDefinedError + 33, ///< @deprecated
    ERR_CANNOT_LOGIN = KJob::UserDefinedError + 33,
    ERR_COULD_NOT_STAT = KJob::UserDefinedError + 34, ///< @deprecated
    ERR_CANNOT_STAT = KJob::UserDefinedError + 34,
    ERR_COULD_NOT_CLOSEDIR = KJob::UserDefinedError + 35, ///< @deprecated
    ERR_CANNOT_CLOSEDIR = KJob::UserDefinedError + 35,
    ERR_COULD_NOT_MKDIR = KJob::UserDefinedError + 37, ///< @deprecated
    ERR_CANNOT_MKDIR = KJob::UserDefinedError + 37,
    ERR_COULD_NOT_RMDIR = KJob::UserDefinedError + 38, ///< @deprecated
    ERR_CANNOT_RMDIR = KJob::UserDefinedError + 38,
    ERR_CANNOT_RESUME = KJob::UserDefinedError + 39,
    ERR_CANNOT_RENAME = KJob::UserDefinedError + 40,
    ERR_CANNOT_CHMOD = KJob::UserDefinedError + 41,
    ERR_CANNOT_DELETE = KJob::UserDefinedError + 42,
    // The text argument is the protocol that the dead slave supported.
    // This means for example: file, ftp, http, ...
    ERR_SLAVE_DIED = KJob::UserDefinedError + 43,
    ERR_OUT_OF_MEMORY = KJob::UserDefinedError + 44,
    ERR_UNKNOWN_PROXY_HOST = KJob::UserDefinedError + 45,
    ERR_COULD_NOT_AUTHENTICATE = KJob::UserDefinedError + 46, ///< @deprecated
    ERR_CANNOT_AUTHENTICATE = KJob::UserDefinedError + 46,
    ERR_ABORTED = KJob::UserDefinedError + 47, ///< Action got aborted from application side
    ERR_INTERNAL_SERVER = KJob::UserDefinedError + 48,
    ERR_SERVER_TIMEOUT = KJob::UserDefinedError + 49,
    ERR_SERVICE_NOT_AVAILABLE = KJob::UserDefinedError + 50,
    ERR_UNKNOWN = KJob::UserDefinedError + 51,
    // (was a warning) ERR_CHECKSUM_MISMATCH = 52,
    ERR_UNKNOWN_INTERRUPT = KJob::UserDefinedError + 53,
    ERR_CANNOT_DELETE_ORIGINAL = KJob::UserDefinedError + 54,
    ERR_CANNOT_DELETE_PARTIAL = KJob::UserDefinedError + 55,
    ERR_CANNOT_RENAME_ORIGINAL = KJob::UserDefinedError + 56,
    ERR_CANNOT_RENAME_PARTIAL = KJob::UserDefinedError + 57,
    ERR_NEED_PASSWD = KJob::UserDefinedError + 58,
    ERR_CANNOT_SYMLINK = KJob::UserDefinedError + 59,
    ERR_NO_CONTENT = KJob::UserDefinedError + 60, ///< Action succeeded but no content will follow.
    ERR_DISK_FULL = KJob::UserDefinedError + 61,
    ERR_IDENTICAL_FILES = KJob::UserDefinedError + 62, ///< src==dest when moving/copying
    ERR_SLAVE_DEFINED = KJob::UserDefinedError + 63, ///< for slave specified errors that can be
    ///< rich text.  Email links will be handled
    ///< by the standard email app and all hrefs
    ///< will be handled by the standard browser.
    ///< <a href="exec:/khelpcenter ?" will be
    ///< forked.
    ERR_UPGRADE_REQUIRED = KJob::UserDefinedError + 64, ///< A transport upgrade is required to access this
    ///< object.  For instance, TLS is demanded by
    ///< the server in order to continue.
    ERR_POST_DENIED = KJob::UserDefinedError + 65, ///< Issued when trying to POST data to a certain Ports
    ERR_COULD_NOT_SEEK = KJob::UserDefinedError + 66, ///< @deprecated
    // see job.cpp
    ERR_CANNOT_SEEK = KJob::UserDefinedError + 66,
    ERR_CANNOT_SETTIME = KJob::UserDefinedError + 67, ///< Emitted by setModificationTime
    ERR_CANNOT_CHOWN = KJob::UserDefinedError + 68,
    ERR_POST_NO_SIZE = KJob::UserDefinedError + 69,
    ERR_DROP_ON_ITSELF = KJob::UserDefinedError + 70, ///< from KIO::DropJob, @since 5.6
    ERR_CANNOT_MOVE_INTO_ITSELF = KJob::UserDefinedError + 71, ///< emitted by KIO::move, @since 5.18
    ERR_PASSWD_SERVER = KJob::UserDefinedError + 72, ///< returned by SlaveBase::openPasswordDialogV2, @since 5.24
    ERR_CANNOT_CREATE_SLAVE = KJob::UserDefinedError + 73 ///< used by Slave::createSlave, @since 5.30
};

/**
 * Specifies how to use the cache.
 * @see parseCacheControl()
 * @see getCacheControlString()
 */
enum CacheControl {
    CC_CacheOnly, ///< Fail request if not in cache
    CC_Cache,     ///< Use cached entry if available
    CC_Verify,    ///< Validate cached entry with remote site if expired
    CC_Refresh,   ///< Always validate cached entry with remote site
    CC_Reload     ///< Always fetch from remote site.
};

/**
 * Specifies privilege file operation status.
 * @since 5.43
 */
enum  PrivilegeOperationStatus {
    OperationAllowed = 1,
    OperationCanceled,
    OperationNotAllowed
};

/**
 * Returns a string representation of the given cache control method.
 *
 * @param cacheControl the cache control method
 * @return the string representation
 * @see parseCacheControl()
 */
KIOCORE_EXPORT QString getCacheControlString(KIO::CacheControl cacheControl);

}

#endif
