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

#include "qpdfpagerenderer.h"

#include <private/qobject_p.h>
#include <QMutex>
#include <QPdfDocument>
#include <QPointer>
#include <QThread>

QT_BEGIN_NAMESPACE

class RenderWorker : public QObject
{
    Q_OBJECT

public:
    RenderWorker();
    ~RenderWorker();

    void setDocument(QPdfDocument *document);

public Q_SLOTS:
    void requestPage(quint64 requestId, int page, QSize imageSize,
                     QPdfDocumentRenderOptions options);

Q_SIGNALS:
    void pageRendered(int page, QSize imageSize, const QImage &image,
                      QPdfDocumentRenderOptions options, quint64 requestId);

private:
    QPointer<QPdfDocument> m_document;
    QMutex m_mutex;
};

class QPdfPageRendererPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QPdfPageRenderer)

public:
    QPdfPageRendererPrivate();
    ~QPdfPageRendererPrivate();

    void handleNextRequest();
    void requestFinished(int page, QSize imageSize, const QImage &image,
                         QPdfDocumentRenderOptions options, quint64 requestId);

    QPdfPageRenderer::RenderMode m_renderMode = QPdfPageRenderer::RenderMode::SingleThreaded;
    QPointer<QPdfDocument> m_document;

    struct PageRequest
    {
        quint64 id;
        int pageNumber;
        QSize imageSize;
        QPdfDocumentRenderOptions options;
    };

    QVector<PageRequest> m_requests;
    QVector<PageRequest> m_pendingRequests;
    quint64 m_requestIdCounter = 1;

    QThread *m_renderThread = nullptr;
    QScopedPointer<RenderWorker> m_renderWorker;
};

Q_DECLARE_TYPEINFO(QPdfPageRendererPrivate::PageRequest, Q_PRIMITIVE_TYPE);


RenderWorker::RenderWorker()
    : m_document(nullptr)
{
}

RenderWorker::~RenderWorker()
{
}

void RenderWorker::setDocument(QPdfDocument *document)
{
    const QMutexLocker locker(&m_mutex);

    if (m_document == document)
        return;

    m_document = document;
}

void RenderWorker::requestPage(quint64 requestId, int pageNumber, QSize imageSize,
                               QPdfDocumentRenderOptions options)
{
    const QMutexLocker locker(&m_mutex);

    if (!m_document || m_document->status() != QPdfDocument::Ready)
        return;

    const QImage image = m_document->render(pageNumber, imageSize, options);

    emit pageRendered(pageNumber, imageSize, image, options, requestId);
}


QPdfPageRendererPrivate::QPdfPageRendererPrivate()
    : QObjectPrivate()
    , m_renderWorker(new RenderWorker)
{
}

QPdfPageRendererPrivate::~QPdfPageRendererPrivate()
{
    if (m_renderThread) {
        m_renderThread->quit();
        m_renderThread->wait();
    }
}

void QPdfPageRendererPrivate::handleNextRequest()
{
    if (m_requests.isEmpty())
        return;

    const PageRequest request = m_requests.takeFirst();
    m_pendingRequests.append(request);

    QMetaObject::invokeMethod(m_renderWorker.data(), "requestPage", Qt::QueuedConnection,
                              Q_ARG(quint64, request.id), Q_ARG(int, request.pageNumber),
                              Q_ARG(QSize, request.imageSize), Q_ARG(QPdfDocumentRenderOptions,
                              request.options));
}

void QPdfPageRendererPrivate::requestFinished(int page, QSize imageSize, const QImage &image, QPdfDocumentRenderOptions options, quint64 requestId)
{
    Q_UNUSED(image);
    Q_UNUSED(requestId);
    const auto it = std::find_if(m_pendingRequests.begin(), m_pendingRequests.end(),
                                 [page, imageSize, options](const PageRequest &request){ return request.pageNumber == page && request.imageSize == imageSize && request.options == options; });

    if (it != m_pendingRequests.end())
        m_pendingRequests.erase(it);
}

/*!
    \class QPdfPageRenderer
    \since 5.11
    \inmodule QtPdf

    \brief The QPdfPageRenderer class encapsulates the rendering of pages of a PDF document.

    The QPdfPageRenderer contains a queue that collects all render requests that are invoked through
    requestPage(). Depending on the configured RenderMode the QPdfPageRenderer processes this queue
    in the main UI thread on next event loop invocation (\c RenderMode::SingleThreaded) or in a separate worker thread
    (\c RenderMode::MultiThreaded) and emits the result through the pageRendered() signal for each request once
    the rendering is done.

    \sa QPdfDocument
*/


/*!
    Constructs a page renderer object with parent object \a parent.
*/
QPdfPageRenderer::QPdfPageRenderer(QObject *parent)
    : QObject(*new QPdfPageRendererPrivate(), parent)
{
    Q_D(QPdfPageRenderer);

    qRegisterMetaType<QPdfDocumentRenderOptions>();

    connect(d->m_renderWorker.data(), &RenderWorker::pageRendered, this,
            [this,d](int page, QSize imageSize, const QImage &image, QPdfDocumentRenderOptions options, quint64 requestId) {
                d->requestFinished(page, imageSize, image, options, requestId);
                emit pageRendered(page, imageSize, image, options, requestId);
                d->handleNextRequest();
           });
}

/*!
    Destroys the page renderer object.
*/
QPdfPageRenderer::~QPdfPageRenderer()
{
}

/*!
    \enum QPdfPageRenderer::RenderMode

    This enum describes how the pages are rendered.

    \value MultiThreaded All pages are rendered in a separate worker thread.
    \value SingleThreaded All pages are rendered in the main UI thread (default).

    \sa renderMode(), setRenderMode()
*/

/*!
    \property QPdfPageRenderer::renderMode
    \brief The mode the renderer uses to render the pages.

    By default, this property is \c RenderMode::SingleThreaded.

    \sa setRenderMode(), RenderMode
*/

/*!
    Returns the mode of how the pages are rendered.

    \sa RenderMode
*/
QPdfPageRenderer::RenderMode QPdfPageRenderer::renderMode() const
{
    Q_D(const QPdfPageRenderer);

    return d->m_renderMode;
}

/*!
    Sets the mode of how the pages are rendered to \a mode.

    \sa RenderMode
*/
void QPdfPageRenderer::setRenderMode(RenderMode mode)
{
    Q_D(QPdfPageRenderer);

    if (d->m_renderMode == mode)
        return;

    d->m_renderMode = mode;
    emit renderModeChanged(d->m_renderMode);

    if (d->m_renderMode == RenderMode::MultiThreaded) {
        d->m_renderThread = new QThread;
        d->m_renderWorker->moveToThread(d->m_renderThread);
        d->m_renderThread->start();
    } else {
        d->m_renderThread->quit();
        d->m_renderThread->wait();
        delete d->m_renderThread;
        d->m_renderThread = nullptr;

        // pulling the object from another thread should be fine, once that thread is deleted
        d->m_renderWorker->moveToThread(this->thread());
    }
}

/*!
    \property QPdfPageRenderer::document
    \brief The document instance this object renders the pages from.

    By default, this property is \c nullptr.

    \sa document(), setDocument(), QPdfDocument
*/

/*!
    Returns the document this objects renders the pages from, or a \c nullptr
    if none has been set before.

    \sa QPdfDocument
*/
QPdfDocument* QPdfPageRenderer::document() const
{
    Q_D(const QPdfPageRenderer);

    return d->m_document;
}

/*!
    Sets the \a document this object renders the pages from.

    \sa QPdfDocument
*/
void QPdfPageRenderer::setDocument(QPdfDocument *document)
{
    Q_D(QPdfPageRenderer);

    if (d->m_document == document)
        return;

    d->m_document = document;
    emit documentChanged(d->m_document);

    d->m_renderWorker->setDocument(d->m_document);
}

/*!
    Requests the renderer to render the page \a pageNumber into a QImage of size \a imageSize
    according to the provided \a options.

    Once the rendering is done the pageRendered() signal is emitted with the result as parameters.

    The return value is an ID that uniquely identifies the render request. If a request with the
    same parameters is still in the queue, the ID of that queued request is returned.
*/
quint64 QPdfPageRenderer::requestPage(int pageNumber, QSize imageSize,
                                      QPdfDocumentRenderOptions options)
{
    Q_D(QPdfPageRenderer);

    if (!d->m_document || d->m_document->status() != QPdfDocument::Ready)
        return 0;

    for (const auto &request : qAsConst(d->m_pendingRequests)) {
        if (request.pageNumber == pageNumber
            && request.imageSize == imageSize
            && request.options == options)
            return request.id;
    }

    const auto id = d->m_requestIdCounter++;

    QPdfPageRendererPrivate::PageRequest request;
    request.id = id;
    request.pageNumber = pageNumber;
    request.imageSize = imageSize;
    request.options = options;

    d->m_requests.append(request);

    d->handleNextRequest();

    return id;
}

QT_END_NAMESPACE

#include "qpdfpagerenderer.moc"
