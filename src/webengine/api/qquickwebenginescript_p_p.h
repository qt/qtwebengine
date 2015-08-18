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

#ifndef QQUICKWEBENGINESCRIPT_P_P_H
#define QQUICKWEBENGINESCRIPT_P_P_H

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

#include "qquickwebenginescript_p.h"

#include <QtCore/QBasicTimer>
#include "user_script.h"
#include "web_contents_adapter.h"

namespace QtWebEngineCore {
class UserScriptControllerHost;
class WebContentsAdapter;
} // namespace

QT_BEGIN_NAMESPACE

class QQuickWebEngineScriptPrivate {
public:
    Q_DECLARE_PUBLIC(QQuickWebEngineScript)
    QQuickWebEngineScriptPrivate();
    void aboutToUpdateUnderlyingScript();
    void bind(QtWebEngineCore::UserScriptControllerHost *, QtWebEngineCore::WebContentsAdapter * = 0);

    QtWebEngineCore::UserScript coreScript;
    QBasicTimer m_basicTimer;
    QtWebEngineCore::UserScriptControllerHost *m_controllerHost;
    QtWebEngineCore::WebContentsAdapter *m_adapter;
    QUrl m_sourceUrl;

private:
    QQuickWebEngineScript *q_ptr;

};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINESCRIPT_P_P_H
