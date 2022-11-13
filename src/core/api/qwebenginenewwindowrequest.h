// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINENEWWINDOWREQUEST_H
#define QWEBENGINENEWWINDOWREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qrect.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qurl.h>

namespace QtWebEngineCore {
class WebContentsAdapter;
}

QT_BEGIN_NAMESPACE

class QWebEnginePage;
struct QWebEngineNewWindowRequestPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineNewWindowRequest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DestinationType destination READ destination CONSTANT FINAL)
    Q_PROPERTY(QUrl requestedUrl READ requestedUrl CONSTANT FINAL)
    Q_PROPERTY(QRect requestedGeometry READ requestedGeometry CONSTANT FINAL)
    Q_PROPERTY(bool userInitiated READ isUserInitiated CONSTANT FINAL)
public:
    ~QWebEngineNewWindowRequest();

    enum DestinationType {
        InNewWindow,
        InNewTab,
        InNewDialog,
        InNewBackgroundTab
    };
    Q_ENUM(DestinationType)

    DestinationType destination() const;
    QUrl requestedUrl() const;
    QRect requestedGeometry() const;
    bool isUserInitiated() const;

    void openIn(QWebEnginePage *);

protected:
    QWebEngineNewWindowRequest(DestinationType, const QRect &, const QUrl &, bool,
                               QSharedPointer<QtWebEngineCore::WebContentsAdapter>,
                               QObject * = nullptr);

    QScopedPointer<QWebEngineNewWindowRequestPrivate> d_ptr;
    friend class QWebEnginePage;
    friend class QWebEnginePagePrivate;
    friend class QQuickWebEngineView;
    friend class QQuickWebEngineViewPrivate;
};

QT_END_NAMESPACE

#endif // QWEBENGINENEWWINDOWREQUEST_H
