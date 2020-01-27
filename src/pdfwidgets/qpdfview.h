/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QPDFVIEW_H
#define QPDFVIEW_H

#include <QtPdfWidgets/qtpdfwidgetsglobal.h>
#include <QtWidgets/qabstractscrollarea.h>

QT_BEGIN_NAMESPACE

class QPdfDocument;
class QPdfPageNavigation;
class QPdfViewPrivate;

class Q_PDF_WIDGETS_EXPORT QPdfView : public QAbstractScrollArea
{
    Q_OBJECT

    Q_PROPERTY(QPdfDocument* document READ document WRITE setDocument NOTIFY documentChanged)

    Q_PROPERTY(PageMode pageMode READ pageMode WRITE setPageMode NOTIFY pageModeChanged)
    Q_PROPERTY(ZoomMode zoomMode READ zoomMode WRITE setZoomMode NOTIFY zoomModeChanged)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor NOTIFY zoomFactorChanged)

    Q_PROPERTY(int pageSpacing READ pageSpacing WRITE setPageSpacing NOTIFY pageSpacingChanged)
    Q_PROPERTY(QMargins documentMargins READ documentMargins WRITE setDocumentMargins NOTIFY documentMarginsChanged)

public:
    enum PageMode
    {
        SinglePage,
        MultiPage
    };
    Q_ENUM(PageMode)

    enum ZoomMode
    {
        CustomZoom,
        FitToWidth,
        FitInView
    };
    Q_ENUM(ZoomMode)

    explicit QPdfView(QWidget *parent = nullptr);
    ~QPdfView();

    void setDocument(QPdfDocument *document);
    QPdfDocument *document() const;

    QPdfPageNavigation *pageNavigation() const;

    PageMode pageMode() const;
    ZoomMode zoomMode() const;
    qreal zoomFactor() const;

    int pageSpacing() const;
    void setPageSpacing(int spacing);

    QMargins documentMargins() const;
    void setDocumentMargins(QMargins margins);

public Q_SLOTS:
    void setPageMode(PageMode mode);
    void setZoomMode(ZoomMode mode);
    void setZoomFactor(qreal factor);

Q_SIGNALS:
    void documentChanged(QPdfDocument *document);
    void pageModeChanged(PageMode pageMode);
    void zoomModeChanged(ZoomMode zoomMode);
    void zoomFactorChanged(qreal zoomFactor);
    void pageSpacingChanged(int pageSpacing);
    void documentMarginsChanged(QMargins documentMargins);

protected:
    explicit QPdfView(QPdfViewPrivate &, QWidget *);

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;

private:
    Q_DECLARE_PRIVATE(QPdfView)
};

QT_END_NAMESPACE

#endif // QPDFVIEW_H
