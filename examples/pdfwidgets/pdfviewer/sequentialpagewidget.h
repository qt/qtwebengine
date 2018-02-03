/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef SEQUENTIALPAGEWIDGET_H
#define SEQUENTIALPAGEWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPdfDocument;
QT_END_NAMESPACE

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

    void setDocument(QPdfDocument *document);

public slots:
    void setZoom(qreal factor);
    void invalidate();

signals:
    void showingPageRange(int start, int end);
    void zoomChanged(qreal factor);

private slots:
    void documentStatusChanged();
    void pageLoaded(int page, qreal zoom, QImage image);

private:
    int pageCount();
    QSizeF pageSize(int page);
    void render(int page);

private:
    QHash<int, QImage> m_pageCache;
    QVector<int> m_cachedPagesLRU;
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

    QPdfDocument *m_document;
};

#endif // SEQUENTIALPAGEWIDGET_H
