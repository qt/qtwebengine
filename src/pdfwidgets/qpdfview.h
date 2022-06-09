// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFVIEW_H
#define QPDFVIEW_H

#include <QtPdfWidgets/qtpdfwidgetsglobal.h>
#include <QtWidgets/qabstractscrollarea.h>

QT_BEGIN_NAMESPACE

class QPdfDocument;
class QPdfPageNavigator;
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
    enum class PageMode
    {
        SinglePage,
        MultiPage
    };
    Q_ENUM(PageMode)

    enum class ZoomMode
    {
        Custom,
        FitToWidth,
        FitInView
    };
    Q_ENUM(ZoomMode)

    QPdfView() : QPdfView(nullptr) {}
    explicit QPdfView(QWidget *parent);
    ~QPdfView();

    void setDocument(QPdfDocument *document);
    QPdfDocument *document() const;

    QPdfPageNavigator *pageNavigator() const;

    PageMode pageMode() const;
    ZoomMode zoomMode() const;
    qreal zoomFactor() const;

    int pageSpacing() const;
    void setPageSpacing(int spacing);

    QMargins documentMargins() const;
    void setDocumentMargins(QMargins margins);

public Q_SLOTS:
    void setPageMode(QPdfView::PageMode mode);
    void setZoomMode(QPdfView::ZoomMode mode);
    void setZoomFactor(qreal factor);

Q_SIGNALS:
    void documentChanged(QPdfDocument *document);
    void pageModeChanged(QPdfView::PageMode pageMode);
    void zoomModeChanged(QPdfView::ZoomMode zoomMode);
    void zoomFactorChanged(qreal zoomFactor);
    void pageSpacingChanged(int pageSpacing);
    void documentMarginsChanged(QMargins documentMargins);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;

private:
    Q_DECLARE_PRIVATE(QPdfView)
    QScopedPointer<QPdfViewPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QPDFVIEW_H
