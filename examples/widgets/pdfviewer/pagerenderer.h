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

#ifndef PAGECACHE_H
#define PAGECACHE_H

#include <QBrush>
#include <QHash>
#include <QPixmap>
#include <QRunnable>
#include <QThread>
#include <QPdfDocument>

class QPdfDocument;

class PageRenderer : public QThread
{
    Q_OBJECT
public:
    PageRenderer();
    ~PageRenderer();

public slots:
    QVector<QSizeF> openDocument(const QUrl &location);
    void requestPage(int page, qreal zoom, Priority priority = QThread::NormalPriority);

signals:
    void pageReady(int page, qreal zoom, QImage image);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    void renderPage(int page, qreal zoom);

private:
    QPdfDocument m_doc;

    // current request only
    int m_page;
    qreal m_zoom;

    // performance statistics
    qreal m_minRenderTime;
    qreal m_maxRenderTime;
    qreal m_totalRenderTime;
    int m_totalPagesRendered;
};

#endif // PAGECACHE_H
