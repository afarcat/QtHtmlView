#include <QtCore/qglobal.h>

#if defined(KHTML_LIBRARY)
#  define KHTML_EXPORT Q_DECL_EXPORT
#else
#  define KHTML_EXPORT Q_DECL_IMPORT
#endif

#define KHTML_NO_EXPORT

#define KHTML_DEPRECATED

#define KHTML_NO_WALLET

#define i18n  QObject::tr

#define i18nc(x, y)  QObject::tr(y)

// define this for keep khtml compatible
#define QHTMLPart           KHTMLPart
#define QHTMLPartPrivate    KHTMLPartPrivate
#define QHTMLPartAccessor   KHTMLPartAccessor
#define QHTMLView           KHTMLView
#define QHTMLGlobal         KHTMLGlobal
#define QHTMLSettings       KHTMLSettings
