// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebenginefaviconprovider_p_p.h"

#include "qquickwebengineprofile.h"
#include "qquickwebenginesettings_p.h"
#include "qquickwebengineview_p_p.h"

#include "profile_adapter.h"
#include "web_contents_adapter.h"

#include <QtCore/qmimedatabase.h>
#include <QtGui/qicon.h>
#include <QtGui/qpixmap.h>
#include <QThread>

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

FaviconImageRequester::FaviconImageRequester(const QUrl &imageSource, const QSize &requestedSize)
    : m_imageSource(imageSource), m_requestedSize(requestedSize)
{
}

void FaviconImageRequester::start()
{
    if (!tryNextView()) {
        // There is no non-otr view to access icon database.
        Q_EMIT done(QPixmap());
    }
}

void FaviconImageRequester::iconRequestDone(const QIcon &icon)
{
    if (icon.isNull()) {
        if (!tryNextView()) {
            // Ran out of views.
            Q_EMIT done(QPixmap());
        }
        return;
    }

    Q_EMIT done(extractPixmap(icon, m_requestedSize));
}

bool FaviconImageRequester::tryNextView()
{
    if (auto view = getNextViewForProcessing()) {
        requestFaviconFromDatabase(view);
        return true;
    }

    return false;
}

void FaviconImageRequester::requestFaviconFromDatabase(QPointer<QQuickWebEngineView> view)
{
    QtWebEngineCore::ProfileAdapter *profileAdapter = view->d_ptr->profileAdapter();
    bool touchIconsEnabled = view->profile()->settings()->touchIconsEnabled();
    if (isIconURL(m_imageSource)) {
        profileAdapter->requestIconForIconURL(
                m_imageSource, qMax(m_requestedSize.width(), m_requestedSize.height()),
                touchIconsEnabled, [this](const QIcon &icon, const QUrl &) {
                    QMetaObject::invokeMethod(this, "iconRequestDone", Qt::QueuedConnection,
                                              Q_ARG(const QIcon &, icon));
                });
    } else {
        profileAdapter->requestIconForPageURL(
                m_imageSource, qMax(m_requestedSize.width(), m_requestedSize.height()),
                touchIconsEnabled, [this](const QIcon &icon, const QUrl &, const QUrl &) {
                    QMetaObject::invokeMethod(this, "iconRequestDone", Qt::QueuedConnection,
                                              Q_ARG(const QIcon &, icon));
                });
    }
}

QPointer<QQuickWebEngineView> FaviconImageRequester::getNextViewForProcessing()
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    for (QPointer<QQuickWebEngineView> view : FaviconProviderHelper::instance()->views()) {
        if (view.isNull())
            continue;
        if (view->profile()->isOffTheRecord())
            continue;
        if (m_processedViews.contains(view))
            continue;
        m_processedViews.append(view);
        return view;
    }
    return nullptr;
}

FaviconProviderHelper::FaviconProviderHelper()
{
    moveToThread(qApp->thread());
}

FaviconProviderHelper *FaviconProviderHelper::instance()
{
    static FaviconProviderHelper instance;
    return &instance;
}

void FaviconProviderHelper::attach(QPointer<QQuickWebEngineView> view)
{
    if (!m_views.contains(view))
        m_views.append(view);
}

void FaviconProviderHelper::detach(QPointer<QQuickWebEngineView> view)
{
    m_views.removeAll(view);
}

void FaviconProviderHelper::handleImageRequest(QPointer<FaviconImageResponse> faviconResponse)
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    if (faviconResponse.isNull())
        return;

    if (m_views.isEmpty()) {
        QMetaObject::invokeMethod(faviconResponse, "handleDone", Qt::QueuedConnection,
                                  Q_ARG(QPixmap, QPixmap()));
        return;
    }

    auto view = findViewByImageSource(faviconResponse->imageSource());
    if (view) {
        QIcon icon = view->d_ptr->adapter->icon();
        if (!icon.isNull()) {
            QMetaObject::invokeMethod(
                    faviconResponse, "handleDone", Qt::QueuedConnection,
                    Q_ARG(QPixmap, extractPixmap(icon, faviconResponse->requestedSize())));
            return;
        }
    }
    startFaviconRequest(faviconResponse);
}

QPointer<QQuickWebEngineView> FaviconProviderHelper::findViewByImageSource(const QUrl &imageSource) const
{
    for (QPointer<QQuickWebEngineView> view : m_views) {
        if (view.isNull())
            continue;

        if (isIconURL(imageSource)) {
            if (view->icon() == QQuickWebEngineFaviconProvider::faviconProviderUrl(imageSource)) {
                return view;
            }
        } else if (view->url() == imageSource) {
            return view;
        }
    }

    return nullptr;
}

void FaviconProviderHelper::startFaviconRequest(QPointer<FaviconImageResponse> faviconResponse)
{
    FaviconImageRequester *requester = new FaviconImageRequester(faviconResponse->imageSource(),
                                                                 faviconResponse->requestedSize());

    connect(requester, &FaviconImageRequester::done, [requester, faviconResponse](QPixmap pixmap) {
        QMetaObject::invokeMethod(faviconResponse, "handleDone", Qt::QueuedConnection,
                                  Q_ARG(QPixmap, pixmap));
        requester->deleteLater();
    });

    requester->start();
}

FaviconImageResponse::FaviconImageResponse(const QUrl &imageSource, const QSize &requestedSize)
    : m_imageSource(imageSource), m_requestedSize(requestedSize)
{
}

void FaviconImageResponse::handleDone(QPixmap pixmap)
{
    m_image = pixmap.toImage();
    Q_EMIT finished();
}

QQuickTextureFactory *FaviconImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
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

QQuickWebEngineFaviconProvider::QQuickWebEngineFaviconProvider()
{
    connect(this, &QQuickWebEngineFaviconProvider::imageResponseRequested,
            FaviconProviderHelper::instance(), &FaviconProviderHelper::handleImageRequest);
}

QQuickImageResponse *
QQuickWebEngineFaviconProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    FaviconImageResponse *response = new FaviconImageResponse(QUrl(id), requestedSize);
    emit imageResponseRequested(response);
    return response;
}

QT_END_NAMESPACE

#include "moc_qquickwebenginefaviconprovider_p_p.cpp"
