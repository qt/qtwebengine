// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKWEBENGINEFAVICONPROVIDER_P_P_H
#define QQUICKWEBENGINEFAVICONPROVIDER_P_P_H

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

#include <QtWebEngineQuick/private/qtwebenginequickglobal_p.h>
#include <QtCore/qlist.h>
#include <QtGui/qimage.h>
#include <QtQuick/qquickimageprovider.h>

QT_BEGIN_NAMESPACE

class QQuickWebEngineView;

class FaviconImageResponse : public QQuickImageResponse
{
    Q_OBJECT

public:
    FaviconImageResponse(const QUrl &imageSource, const QSize &requestedSize, bool isIconUrl);

    QQuickTextureFactory *textureFactory() const override;
    const QUrl &imageSource() const { return m_imageSource; }
    const QSize &requestedSize() const { return m_requestedSize; }
    bool isIconUrl() const { return m_isIconUrl; }

public slots:
    void handleDone(QPixmap pixmap);

private:
    QImage m_image;
    QUrl m_imageSource;
    QSize m_requestedSize;
    bool m_isIconUrl;
};

class FaviconImageRequester : public QObject
{
    Q_OBJECT

public:
    FaviconImageRequester(const QUrl &imageSource, const QSize &requestedSize, bool isIconUrl);
    void start();

public slots:
    void iconRequestDone(const QIcon &icon);

signals:
    void done(QPixmap pixmap);

private:
    bool tryNextView();
    void requestFaviconFromDatabase(QPointer<QQuickWebEngineView> view);
    QPointer<QQuickWebEngineView> getNextViewForProcessing();

    QUrl m_imageSource;
    QSize m_requestedSize;
    bool m_isIconUrl;
    QList<QPointer<QQuickWebEngineView>> m_processedViews;
};

class Q_WEBENGINEQUICK_EXPORT QQuickWebEngineFaviconProvider : public QQuickAsyncImageProvider
{
    Q_OBJECT
public:
    static QString identifier();
    static QUrl faviconProviderUrlBase(const QUrl &, const QString &identifier);
    static QUrl faviconProviderUrl(const QUrl &);

    QQuickWebEngineFaviconProvider();
    virtual bool isIconUrl() const { return true; }
    QQuickImageResponse *requestImageResponse(const QString &id,
                                              const QSize &requestedSize) override;

signals:
    void imageResponseRequested(QPointer<FaviconImageResponse> faviconResponse);
};

class Q_WEBENGINEQUICK_EXPORT QQuickWebEngineFaviconDBProvider : public QQuickWebEngineFaviconProvider
{
    Q_OBJECT
public:
    static QString identifier();
    static QUrl faviconProviderUrl(const QUrl &);

    QQuickWebEngineFaviconDBProvider() = default;
    bool isIconUrl() const override { return false; }
};

class Q_WEBENGINEQUICK_EXPORT FaviconProviderHelper : public QObject
{
    Q_OBJECT

public:
    static FaviconProviderHelper *instance();
    void attach(QPointer<QQuickWebEngineView> view);
    void detach(QPointer<QQuickWebEngineView> view);
    const QList<QPointer<QQuickWebEngineView>> &views() const { return m_views; }

public slots:
    void handleImageRequest(QPointer<FaviconImageResponse> faviconResponse);

private:
    FaviconProviderHelper();
    void startFaviconRequest(QPointer<FaviconImageResponse> faviconResponse);
    QPointer<QQuickWebEngineView> findViewByImageSource(const QUrl &imageSource, bool isIconUrl) const;
    QList<QPointer<QQuickWebEngineView>> m_views;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEFAVICONPROVIDER_P_P_H
