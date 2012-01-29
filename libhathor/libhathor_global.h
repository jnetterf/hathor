#ifndef LIBHATHOR_GLOBAL_H
#define LIBHATHOR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBHATHOR_LIBRARY)
#  define LIBHATHORSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBHATHORSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBHATHOR_GLOBAL_H