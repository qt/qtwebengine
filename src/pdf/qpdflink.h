// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFLINK_H
#define QPDFLINK_H

#include <QtPdf/qtpdfglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qshareddata.h>
#include <QtGui/qclipboard.h>

QT_BEGIN_NAMESPACE

class QDebug;
class QPdfLinkPrivate;

class QPdfLink
{
    Q_GADGET_EXPORT(Q_PDF_EXPORT)
    Q_PROPERTY(bool valid READ isValid)
    Q_PROPERTY(int page READ page)
    Q_PROPERTY(QPointF location READ location)
    Q_PROPERTY(qreal zoom READ zoom)
    Q_PROPERTY(QUrl url READ url)
    Q_PROPERTY(QString contextBefore READ contextBefore)
    Q_PROPERTY(QString contextAfter READ contextAfter)
    Q_PROPERTY(QList<QRectF> rectangles READ rectangles)

public:
    Q_PDF_EXPORT QPdfLink();
    Q_PDF_EXPORT ~QPdfLink();
    Q_PDF_EXPORT QPdfLink &operator=(const QPdfLink &other);

    Q_PDF_EXPORT QPdfLink(const QPdfLink &other) noexcept;
    Q_PDF_EXPORT QPdfLink(QPdfLink &&other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPdfLink)

    void swap(QPdfLink &other) noexcept { d.swap(other.d); }

    Q_PDF_EXPORT bool isValid() const;
    Q_PDF_EXPORT int page() const;
    Q_PDF_EXPORT QPointF location() const;
    Q_PDF_EXPORT qreal zoom() const;
    Q_PDF_EXPORT QUrl url() const;
    Q_PDF_EXPORT QString contextBefore() const;
    Q_PDF_EXPORT QString contextAfter() const;
    Q_PDF_EXPORT QList<QRectF> rectangles() const;
    Q_PDF_EXPORT Q_INVOKABLE QString toString() const;
    Q_PDF_EXPORT Q_INVOKABLE void copyToClipboard(QClipboard::Mode mode = QClipboard::Clipboard) const;

private: // methods
    QPdfLink(int page, QPointF location, qreal zoom);
    QPdfLink(int page, QList<QRectF> rects, QString contextBefore, QString contextAfter);
    QPdfLink(QPdfLinkPrivate *d);
    friend class QPdfDocument;
    friend class QPdfLinkModelPrivate;
    friend class QPdfSearchModelPrivate;
    friend class QPdfPageNavigator;
    friend class QQuickPdfPageNavigator;

private: // storage
    QExplicitlySharedDataPointer<QPdfLinkPrivate> d;

};
Q_DECLARE_SHARED(QPdfLink)

#ifndef QT_NO_DEBUG_STREAM
Q_PDF_EXPORT QDebug operator<<(QDebug, const QPdfLink &);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPdfLink)

#endif // QPDFLINK_H
