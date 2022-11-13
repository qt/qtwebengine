// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
#include <QtCore/qrunnable.h>
#include <QtCore/qthreadpool.h>
#include <QtGui/qimage.h>
#include <QtQuick/qquickimageprovider.h>

QT_BEGIN_NAMESPACE

class QQuickWebEngineView;

class FaviconImageResponseRunnable : public QObject, public QRunnable
{
    Q_OBJECT

public:
    FaviconImageResponseRunnable(const QString &id, const QSize &requestedSize,
                                 QList<QQuickWebEngineView *> *views);
    void run() override;
    void iconRequestDone(const QIcon &icon);

signals:
    void done(QPixmap pixmap);

private:
    int tryNextView();
    void requestIconOnUIThread(QQuickWebEngineView *view);

    QString m_id;
    QSize m_requestedSize;
    QList<QQuickWebEngineView *> *m_views;
    int m_nextViewIndex = 0;
};

class FaviconImageResponse : public QQuickImageResponse
{
public:
    FaviconImageResponse();
    FaviconImageResponse(const QString &id, const QSize &requestedSize,
                         QList<QQuickWebEngineView *> *views, QThreadPool *pool);
    ~FaviconImageResponse();
    void handleDone(QPixmap pixmap);
    QQuickTextureFactory *textureFactory() const override;

private:
    void startRunnable(const QString &id, const QSize &requestedSize,
                       QList<QQuickWebEngineView *> *views, QThreadPool *pool);

    FaviconImageResponseRunnable *m_runnable = nullptr;
    QImage m_image;
};

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineFaviconProvider : public QQuickAsyncImageProvider
{
public:
    static QString identifier();
    static QUrl faviconProviderUrl(const QUrl &);

    QQuickWebEngineFaviconProvider();
    ~QQuickWebEngineFaviconProvider();

    void attach(QQuickWebEngineView *view) { m_views.append(view); }
    void detach(QQuickWebEngineView *view) { m_views.removeAll(view); }

    QQuickImageResponse *requestImageResponse(const QString &id,
                                              const QSize &requestedSize) override;

private:
    QThreadPool m_pool;
    QList<QQuickWebEngineView *> m_views;
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEFAVICONPROVIDER_P_P_H
