#ifndef QTPDFGLOBAL_H
#define QTPDFGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef Q_PDF_EXPORT
#  ifndef QT_STATIC
#    if defined(QT_BUILD_PDF_LIB)
#      define Q_PDF_EXPORT Q_DECL_EXPORT
#    else
#      define Q_PDF_EXPORT Q_DECL_IMPORT
#    endif
#  else
#    define Q_PDF_EXPORT
#  endif
#endif

QT_END_NAMESPACE

#endif // QTPDFGLOBAL_H

