// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebenginefaviconprovider_p_p.h"

#include "qquickwebengineprofile.h"
#include "qquickwebenginesettings_p.h"
#include "qquickwebengineview_p_p.h"

#include "profile_adapter.h"
#include "web_contents_adapter.h"

#include <QtCore/qmimedatabase.h>
#include <QtCore/qtimer.h>
#include <QtGui/qicon.h>
#include <QtGui/qpixmap.h>

QT_BEGIN_NAMESPACE

static inline unsigned area(const QSize &size)
{
    return size.width() * size.height();
}

static QSize largestSize(const QList<QSize> &availableSizes)
{
    QSize result;
    for (const QSize &size : availableSizes) {
        if (area(size) > area(result))
            result = size;
    }

    return result;
}

static QSize fitSize(const QList<QSize> &availableSizes, const QSize &requestedSize)
{
    Q_ASSERT(availableSizes.size());
    QSize result = largestSize(availableSizes);
    if (availableSizes.size() == 1 || area(requestedSize) >= area(result))
        return result;

    for (const QSize &size : availableSizes) {
        if (area(size) == area(requestedSize))
            return size;

        if (area(requestedSize) < area(size) && area(size) < area(result))
            result = size;
    }

    return result;
}

static QPixmap extractPixmap(const QIcon &icon, const QSize &requestedSize)
{
    Q_ASSERT(!icon.isNull());

    // If source size is not specified, use the largest icon
    if (!requestedSize.isValid())
        return icon.pixmap(largestSize(icon.availableSizes()), 1.0).copy();

    const QSize &size = fitSize(icon.availableSizes(), requestedSize);
    const QPixmap &iconPixmap = icon.pixmap(size, 1.0);
    return iconPixmap.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation).copy();
}

static bool isIconURL(const QUrl &url)
{
    QMimeType mimeType = QMimeDatabase().mimeTypeForFile(url.path(), QMimeDatabase::MatchExtension);

    // Check file extension.
    if (mimeType.name().startsWith(QLatin1String("image")))
        return true;

    // Check if it is an image data: URL.
    if (url.scheme() == QLatin1String("data") && url.path().startsWith(QLatin1String("image")))
        return true;

    return false;
}

static QQuickWebEngineView *findViewById(const QString &id, QList<QQuickWebEngineView *> *views)
{
    QQuickWebEngineView *result = nullptr;
    for (QQuickWebEngineView *view : *views) {
        if (isIconURL(QUrl(id))) {
            if (view->icon() == QQuickWebEngineFaviconProvider::faviconProviderUrl(QUrl(id))) {
                result = view;
                break;
            }
        } else if (view->url() == QUrl(id)) {
            result = view;
            break;
        }
    }

    return result;
}

FaviconImageResponseRunnable::FaviconImageResponseRunnable(const QString &id,
                                                           const QSize &requestedSize,
                                                           QList<QQuickWebEngineView *> *views)
    : m_id(id), m_requestedSize(requestedSize), m_views(views)
{
}

void FaviconImageResponseRunnable::run()
{
    if (tryNextView() == -1) {
        // There is no non-otr view to access icon database.
        Q_EMIT done(QPixmap());
    }
}

void FaviconImageResponseRunnable::iconRequestDone(const QIcon &icon)
{
    if (icon.isNull()) {
        if (tryNextView() == -1) {
            // Ran out of views.
            Q_EMIT done(QPixmap());
        }
        return;
    }

    Q_EMIT done(extractPixmap(icon, m_requestedSize).copy());
}

int FaviconImageResponseRunnable::tryNextView()
{
    for (; m_nextViewIndex < m_views->size(); ++m_nextViewIndex) {
        QQuickWebEngineView *view = m_views->at(m_nextViewIndex);
        if (view->profile()->isOffTheRecord())
            continue;

        requestIconOnUIThread(view);

        return m_nextViewIndex++;
    }

    return -1;
}

void FaviconImageResponseRunnable::requestIconOnUIThread(QQuickWebEngineView *view)
{
    QTimer *timer = new QTimer();
    timer->moveToThread(qApp->thread());
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [this, view, timer]() {
        QtWebEngineCore::ProfileAdapter *profileAdapter = view->d_ptr->profileAdapter();
        bool touchIconsEnabled = view->profile()->settings()->touchIconsEnabled();
        if (isIconURL(QUrl(m_id))) {
            profileAdapter->requestIconForIconURL(QUrl(m_id),
                                                  qMax(m_requestedSize.width(), m_requestedSize.height()),
                                                  touchIconsEnabled,
                                                  [this](const QIcon &icon, const QUrl &) { iconRequestDone(icon); });
        } else {
            profileAdapter->requestIconForPageURL(QUrl(m_id),
                                                  qMax(m_requestedSize.width(), m_requestedSize.height()),
                                                  touchIconsEnabled,
                                                  [this](const QIcon &icon, const QUrl &, const QUrl &) { iconRequestDone(icon); });
        }
        timer->deleteLater();
    });
    QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
}

FaviconImageResponse::FaviconImageResponse()
{
    Q_EMIT finished();
}

FaviconImageResponse::FaviconImageResponse(const QString &id, const QSize &requestedSize,
                                           QList<QQuickWebEngineView *> *views, QThreadPool *pool)
{
    if (QQuickWebEngineView *view = findViewById(id, views)) {
        QTimer *timer = new QTimer();
        timer->moveToThread(qApp->thread());
        timer->setSingleShot(true);
        QObject::connect(timer, &QTimer::timeout, [this, id, requestedSize, views, pool, view, timer]() {
            QIcon icon = view->d_ptr->adapter->icon();
            if (icon.isNull())
                startRunnable(id, requestedSize, views, pool);
            else
                handleDone(extractPixmap(icon, requestedSize).copy());
            timer->deleteLater();
        });
        QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
    } else {
        startRunnable(id, requestedSize, views, pool);
    }
}

FaviconImageResponse::~FaviconImageResponse() { }

void FaviconImageResponse::handleDone(QPixmap pixmap)
{
    if (m_runnable)
        delete m_runnable;
    m_image = pixmap.toImage();
    Q_EMIT finished();
}

QQuickTextureFactory *FaviconImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

void FaviconImageResponse::startRunnable(const QString &id, const QSize &requestedSize,
                                         QList<QQuickWebEngineView *> *views, QThreadPool *pool)
{
    m_runnable = new FaviconImageResponseRunnable(id, requestedSize, views);
    m_runnable->setAutoDelete(false);
    connect(m_runnable, &FaviconImageResponseRunnable::done, this,
            &FaviconImageResponse::handleDone);
    pool->start(m_runnable);
}

QString QQuickWebEngineFaviconProvider::identifier()
{
    return QStringLiteral("favicon");
}

QUrl QQuickWebEngineFaviconProvider::faviconProviderUrl(const QUrl &url)
{
    if (url.isEmpty())
        return url;

    QUrl providerUrl;
    providerUrl.setScheme(QStringLiteral("image"));
    providerUrl.setHost(identifier());
    providerUrl.setPath(
            QStringLiteral("/%1").arg(url.toString(QUrl::RemoveQuery | QUrl::RemoveFragment)));
    if (url.hasQuery())
        providerUrl.setQuery(url.query(QUrl::FullyDecoded));
    if (url.hasFragment())
        providerUrl.setFragment(url.fragment(QUrl::FullyDecoded));

    return providerUrl;
}

QQuickWebEngineFaviconProvider::QQuickWebEngineFaviconProvider() { }

QQuickWebEngineFaviconProvider::~QQuickWebEngineFaviconProvider() { }

QQuickImageResponse *
QQuickWebEngineFaviconProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    if (m_views.empty())
        return new FaviconImageResponse;

    FaviconImageResponse *response = new FaviconImageResponse(id, requestedSize, &m_views, &m_pool);
    return response;
}

QT_END_NAMESPACE

#include "moc_qquickwebenginefaviconprovider_p_p.cpp"
