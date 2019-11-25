/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef FIND_TEXT_HELPER_H
#define FIND_TEXT_HELPER_H

#include "qtwebenginecoreglobal_p.h"

#include "qwebenginecallback_p.h"
#include <QJSValue>

namespace content {
class WebContents;
}

namespace gfx {
class Rect;
}

namespace QtWebEngineCore {

class WebContentsAdapterClient;

class Q_WEBENGINECORE_PRIVATE_EXPORT FindTextHelper {
public:
    FindTextHelper(content::WebContents *webContents, WebContentsAdapterClient *viewClient);
    ~FindTextHelper();

    void startFinding(const QString &findText, bool caseSensitively, bool findBackward, const QWebEngineCallback<bool> resultCallback);
    void startFinding(const QString &findText, bool caseSensitively, bool findBackward, const QJSValue &resultCallback);
    void startFinding(const QString &findText, bool caseSensitively, bool findBackward);
    void stopFinding();
    bool isFindTextInProgress() const;
    void handleFindReply(content::WebContents *source, int requestId, int numberOfMatches, const gfx::Rect &selectionRect, int activeMatch, bool finalUpdate);
    void handleLoadCommitted();

private:
    void invokeResultCallback(int requestId, int numberOfMatches);

    content::WebContents *m_webContents;
    WebContentsAdapterClient *m_viewClient;

    static int m_findRequestIdCounter;
    int m_currentFindRequestId;
    int m_lastCompletedFindRequestId;

    QString m_previousFindText;

    QMap<int, QJSValue> m_quickCallbacks;
    CallbackDirectory m_widgetCallbacks;
};

} // namespace QtWebEngineCore

#endif // FIND_TEXT_HELPER_H
