#include <QtCore/qglobal.h>

#if defined(KHTML_LIBRARY)
#  define KPARTS_EXPORT Q_DECL_EXPORT
#else
#  define KPARTS_EXPORT Q_DECL_IMPORT
#endif
