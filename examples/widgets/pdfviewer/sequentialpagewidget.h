/******************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt PDF Module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#ifndef SEQUENTIALPAGEWIDGET_H
#define SEQUENTIALPAGEWIDGET_H

#include <QWidget>

class QPdfDocument;
class PageRenderer;

class SequentialPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SequentialPageWidget(QWidget *parent = 0);
    ~SequentialPageWidget();

    void paintEvent(QPaintEvent * event);
    qreal zoom() { return m_zoom; }
    qreal yForPage(int page);
    int topPageShowing() { return m_topPageShowing; }
    int bottomPageShowing() { return m_bottomPageShowing; }

public slots:
    void openDocument(const QUrl &url);
    void setZoom(qreal factor);
    void invalidate();

signals:
    void showingPageRange(int start, int end);
    void zoomChanged(qreal factor);

private slots:
    void pageLoaded(int page, qreal zoom, QImage image);

private:
    int pageCount();
    QSizeF pageSize(int page);
    void render(int page);

private:
    QHash<int, QImage> m_pageCache;
    QList<int> m_cachedPagesLRU;
    int m_pageCacheLimit;
    QVector<QSizeF> m_pageSizes;
    PageRenderer *m_pageRenderer;
    QBrush m_background;
    QPixmap m_placeholderIcon;
    QBrush m_placeholderBackground;
    int m_pageSpacing;
    int m_topPageShowing;
    int m_bottomPageShowing;
    QSize m_totalSize;
    qreal m_zoom;
    qreal m_screenResolution; // pixels per point
};

#endif // SEQUENTIALPAGEWIDGET_H
