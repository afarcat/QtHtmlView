#include <QtCore/qglobal.h>

#if defined(KHTML_LIBRARY)
#  define KIOCORE_EXPORT Q_DECL_EXPORT
#else
#  define KIOCORE_EXPORT Q_DECL_IMPORT
#endif
