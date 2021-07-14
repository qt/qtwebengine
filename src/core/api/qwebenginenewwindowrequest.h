/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
