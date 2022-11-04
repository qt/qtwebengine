// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

#include <QJSValue>
#include <QMap>
#include <QString>

#include <functional>

namespace content {
class WebContents;
}

namespace gfx {
class Rect;
}

QT_FORWARD_DECLARE_CLASS(QWebEngineFindTextResult)

namespace QtWebEngineCore {

class WebContentsAdapterClient;

class Q_WEBENGINECORE_PRIVATE_EXPORT FindTextHelper {
public:
    FindTextHelper(content::WebContents *webContents, WebContentsAdapterClient *viewClient);
    ~FindTextHelper();

    void startFinding(const QString &findText, bool caseSensitively, bool findBackward, const std::function<void(const QWebEngineFindTextResult &)> &resultCallback);
    void startFinding(const QString &findText, bool caseSensitively, bool findBackward, const QJSValue &resultCallback);
    void startFinding(const QString &findText, bool caseSensitively, bool findBackward);
    void stopFinding();
    bool isFindTextInProgress() const;
    void handleFindReply(content::WebContents *source, int requestId, int numberOfMatches, const gfx::Rect &selectionRect, int activeMatch, bool finalUpdate);
    void handleLoadCommitted();

private:
    void invokeResultCallback(int requestId, int numberOfMatches, int activeMatch);

    content::WebContents *m_webContents;
    WebContentsAdapterClient *m_viewClient;

    static int m_findRequestIdCounter;
    int m_currentFindRequestId;
    int m_lastCompletedFindRequestId;

    QString m_previousFindText;

    QMap<int, QJSValue> m_quickCallbacks;
    QMap<int, std::function<void(const QWebEngineFindTextResult &)>> m_widgetCallbacks;
};

} // namespace QtWebEngineCore

#endif // FIND_TEXT_HELPER_H
