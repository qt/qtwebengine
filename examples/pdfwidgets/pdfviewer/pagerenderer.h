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

#ifndef PAGECACHE_H
#define PAGECACHE_H

#include <QImage>
#include <QThread>

QT_BEGIN_NAMESPACE
class QPdfDocument;
QT_END_NAMESPACE

class PageRenderer : public QThread
{
    Q_OBJECT

public:
    explicit PageRenderer(QObject *parent = nullptr);

public slots:
    void setDocument(QPdfDocument *document);

    void requestPage(int page, qreal zoom, Priority priority = QThread::NormalPriority);

signals:
    void pageReady(int page, qreal zoom, QImage image);

protected:
    void run() override;

private:
    void renderPage(int page, qreal zoom);

private:
    QPdfDocument *m_document;

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
