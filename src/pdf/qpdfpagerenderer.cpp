// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Tobias König <tobias.koenig@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpdfpagerenderer.h"

#include <private/qobject_p.h>
#include <QMutex>
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

class QPdfPageRendererPrivate
{
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

    QList<PageRequest> m_requests;
    QList<PageRequest> m_pendingRequests;
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

    if (!m_document || m_document->status() != QPdfDocument::Status::Ready)
        return;

    const QImage image = m_document->render(pageNumber, imageSize, options);

    emit pageRendered(pageNumber, imageSize, image, options, requestId);
}

QPdfPageRendererPrivate::QPdfPageRendererPrivate() : m_renderWorker(new RenderWorker) { }

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
    : QObject(parent), d_ptr(new QPdfPageRendererPrivate)
{
    qRegisterMetaType<QPdfDocumentRenderOptions>();

    connect(d_ptr->m_renderWorker.data(), &RenderWorker::pageRendered, this,
            [this](int page, QSize imageSize, const QImage &image,
                   QPdfDocumentRenderOptions options, quint64 requestId) {
                d_ptr->requestFinished(page, imageSize, image, options, requestId);
                emit pageRendered(page, imageSize, image, options, requestId);
                d_ptr->handleNextRequest();
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
    return d_ptr->m_renderMode;
}

/*!
    Sets the mode of how the pages are rendered to \a mode.

    \sa RenderMode
*/
void QPdfPageRenderer::setRenderMode(RenderMode mode)
{
    if (d_ptr->m_renderMode == mode)
        return;

    d_ptr->m_renderMode = mode;
    emit renderModeChanged(d_ptr->m_renderMode);

    if (d_ptr->m_renderMode == RenderMode::MultiThreaded) {
        d_ptr->m_renderThread = new QThread;
        d_ptr->m_renderWorker->moveToThread(d_ptr->m_renderThread);
        d_ptr->m_renderThread->start();
    } else {
        d_ptr->m_renderThread->quit();
        d_ptr->m_renderThread->wait();
        delete d_ptr->m_renderThread;
        d_ptr->m_renderThread = nullptr;

        // pulling the object from another thread should be fine, once that thread is deleted
        d_ptr->m_renderWorker->moveToThread(this->thread());
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
    return d_ptr->m_document;
}

/*!
    Sets the \a document this object renders the pages from.

    \sa QPdfDocument
*/
void QPdfPageRenderer::setDocument(QPdfDocument *document)
{
    if (d_ptr->m_document == document)
        return;

    d_ptr->m_document = document;
    emit documentChanged(d_ptr->m_document);

    d_ptr->m_renderWorker->setDocument(d_ptr->m_document);
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
    if (!d_ptr->m_document || d_ptr->m_document->status() != QPdfDocument::Status::Ready)
        return 0;

    for (const auto &request : std::as_const(d_ptr->m_pendingRequests)) {
        if (request.pageNumber == pageNumber
            && request.imageSize == imageSize
            && request.options == options)
            return request.id;
    }

    const auto id = d_ptr->m_requestIdCounter++;

    QPdfPageRendererPrivate::PageRequest request;
    request.id = id;
    request.pageNumber = pageNumber;
    request.imageSize = imageSize;
    request.options = options;

    d_ptr->m_requests.append(request);

    d_ptr->handleNextRequest();

    return id;
}

QT_END_NAMESPACE

#include "qpdfpagerenderer.moc"
#include "moc_qpdfpagerenderer.cpp"
