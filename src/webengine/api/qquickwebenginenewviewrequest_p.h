/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
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

#ifndef QQUICKWEBENGINENEWVIEWREQUEST_P_H
#define QQUICKWEBENGINENEWVIEWREQUEST_P_H

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

#include "qtwebengineglobal_p.h"
#include "qquickwebengineview_p.h"

namespace QtWebEngineCore {
class WebContentsAdapter;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineNewViewRequest : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQuickWebEngineView::NewViewDestination destination READ destination CONSTANT FINAL)
    Q_PROPERTY(bool userInitiated READ isUserInitiated CONSTANT FINAL)
public:
    ~QQuickWebEngineNewViewRequest();

    QQuickWebEngineView::NewViewDestination destination() const;
    bool isUserInitiated() const;
    Q_INVOKABLE void openIn(QQuickWebEngineView *view);

private:
    QQuickWebEngineNewViewRequest();
    QQuickWebEngineView::NewViewDestination m_destination;
    bool m_isUserInitiated;
    QSharedPointer<QtWebEngineCore::WebContentsAdapter> m_adapter;
    QUrl m_requestedUrl;
    friend class QQuickWebEngineView;
    friend class QQuickWebEngineViewPrivate;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickWebEngineNewViewRequest)

#endif // QQUICKWEBENGINENEWVIEWREQUEST_P_H
