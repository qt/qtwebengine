// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPDFVIEW_P_H
#define QPDFVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qpdfview.h"
#include "qpdfdocument.h"
#include "qpdflinkmodel.h"

#include <QHash>
#include <QPointer>

QT_BEGIN_NAMESPACE

class QPdfPageRenderer;

class QPdfViewPrivate
{
    Q_DECLARE_PUBLIC(QPdfView)

public:
    QPdfViewPrivate(QPdfView *q);
    void init();

    void documentStatusChanged();
    void currentPageChanged(int currentPage);
    void calculateViewport();
    void setViewport(QRect viewport);
    void updateScrollBars();

    void pageRendered(int pageNumber, QSize imageSize, const QImage &image, quint64 requestId);
    void invalidateDocumentLayout();
    void invalidatePageCache();

    qreal yPositionForPage(int page) const;

    QTransform screenScaleTransform() const; // points to pixels

    struct DocumentLayout
    {
        QSize documentSize;
        QHash<int, QRect> pageGeometries;
    };

    DocumentLayout calculateDocumentLayout() const;
    void updateDocumentLayout();

    QPdfView *q_ptr;
    QPointer<QPdfDocument> m_document;
    QPointer<QPdfSearchModel> m_searchModel;
    QPdfPageNavigator* m_pageNavigator;
    QPdfPageRenderer *m_pageRenderer;
    QPdfLinkModel m_linkModel;

    QPdfView::PageMode m_pageMode;
    QPdfView::ZoomMode m_zoomMode;
    qreal m_zoomFactor;

    int m_currentSearchResultIndex = -1;

    int m_pageSpacing;
    QMargins m_documentMargins;

    bool m_blockPageScrolling;

    QMetaObject::Connection m_documentStatusChangedConnection;

    QRect m_viewport;

    QHash<int, QImage> m_pageCache;
    QList<int> m_cachedPagesLRU;
    int m_pageCacheLimit;

    DocumentLayout m_documentLayout;

    qreal m_screenResolution; // pixels per point
};

Q_DECLARE_TYPEINFO(QPdfViewPrivate::DocumentLayout, Q_RELOCATABLE_TYPE);

QT_END_NAMESPACE

#endif // QPDFVIEW_P_H
