#include "global.h"

#include <QUrl>

QString KIO::getCacheControlString(KIO::CacheControl cacheControl)
{
    if (cacheControl == KIO::CC_CacheOnly) {
        return QStringLiteral("CacheOnly");
    }
    if (cacheControl == KIO::CC_Cache) {
        return QStringLiteral("Cache");
    }
    if (cacheControl == KIO::CC_Verify) {
        return QStringLiteral("Verify");
    }
    if (cacheControl == KIO::CC_Refresh) {
        return QStringLiteral("Refresh");
    }
    if (cacheControl == KIO::CC_Reload) {
        return QStringLiteral("Reload");
    }
    qCDebug(KIO_CORE) << "unrecognized Cache control enum value:" << cacheControl;
    return QString();
}
