#ifndef TABLE_GLOBAL_H
#define TABLE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TABLEWIDGET_LIBRARY)
#define TABLE_EXPORT Q_DECL_EXPORT
#else
#define TABLE_EXPORT Q_DECL_IMPORT
#endif

#endif  // TABLE_GLOBAL_H
