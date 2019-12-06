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

#include "find_text_helper.h"
#include "qwebenginefindtextresult.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"

#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/mojom/frame/find_in_page.mojom.h"

namespace QtWebEngineCore {

// static
int FindTextHelper::m_findRequestIdCounter = -1;

FindTextHelper::FindTextHelper(content::WebContents *webContents, WebContentsAdapterClient *viewClient)
    : m_webContents(webContents)
    , m_viewClient(viewClient)
    , m_currentFindRequestId(m_findRequestIdCounter++)
    , m_lastCompletedFindRequestId(m_currentFindRequestId)
{
}

FindTextHelper::~FindTextHelper()
{
    if (isFindTextInProgress())
        stopFinding();
}

void FindTextHelper::startFinding(const QString &findText, bool caseSensitively, bool findBackward, const QWebEngineCallback<bool> resultCallback)
{
    if (findText.isEmpty()) {
        stopFinding();
        m_viewClient->findTextFinished(QWebEngineFindTextResult());
        m_widgetCallbacks.invokeEmpty(resultCallback);
        return;
    }

    startFinding(findText, caseSensitively, findBackward);
    m_widgetCallbacks.registerCallback(m_currentFindRequestId, resultCallback);
}

void FindTextHelper::startFinding(const QString &findText, bool caseSensitively, bool findBackward, const QJSValue &resultCallback)
{
    if (findText.isEmpty()) {
        stopFinding();
        m_viewClient->findTextFinished(QWebEngineFindTextResult());
        if (!resultCallback.isUndefined()) {
            QJSValueList args;
            args.append(QJSValue(0));
            const_cast<QJSValue&>(resultCallback).call(args);
        }
        return;
    }

    startFinding(findText, caseSensitively, findBackward);
    if (!resultCallback.isUndefined())
        m_quickCallbacks.insert(m_currentFindRequestId, resultCallback);
}

void FindTextHelper::startFinding(const QString &findText, bool caseSensitively, bool findBackward)
{
    if (findText.isEmpty()) {
        stopFinding();
        return;
    }

    if (m_currentFindRequestId > m_lastCompletedFindRequestId) {
        // There are cases where the render process will overwrite a previous request
        // with the new search and we'll have a dangling callback, leaving the application
        // waiting for it forever.
        // Assume that any unfinished find has been unsuccessful when a new one is started
        // to cover that case.
        m_lastCompletedFindRequestId = m_currentFindRequestId;
        m_viewClient->findTextFinished(QWebEngineFindTextResult());
        invokeResultCallback(m_currentFindRequestId, 0);
    }

    blink::mojom::FindOptionsPtr options = blink::mojom::FindOptions::New();
    options->forward = !findBackward;
    options->match_case = caseSensitively;
    options->find_next = findText == m_previousFindText;
    m_previousFindText = findText;

    m_currentFindRequestId = m_findRequestIdCounter++;
    m_webContents->Find(m_currentFindRequestId, toString16(findText), std::move(options));
}

void FindTextHelper::stopFinding()
{
    m_lastCompletedFindRequestId = m_currentFindRequestId;
    m_previousFindText = QString();
    m_webContents->StopFinding(content::STOP_FIND_ACTION_KEEP_SELECTION);
}

bool FindTextHelper::isFindTextInProgress() const
{
    return m_currentFindRequestId != m_lastCompletedFindRequestId;
}

void FindTextHelper::handleFindReply(content::WebContents *source, int requestId, int numberOfMatches,
                                     const gfx::Rect &selectionRect, int activeMatch, bool finalUpdate)
{
    Q_UNUSED(selectionRect);

    Q_ASSERT(source == m_webContents);

    if (!finalUpdate || requestId <= m_lastCompletedFindRequestId)
        return;

    Q_ASSERT(m_currentFindRequestId == requestId);
    m_lastCompletedFindRequestId = requestId;
    m_viewClient->findTextFinished(QWebEngineFindTextResult(numberOfMatches, activeMatch));
    invokeResultCallback(requestId, numberOfMatches);
}

void FindTextHelper::handleLoadCommitted()
{
    // Make sure that we don't set the findNext WebFindOptions on a new frame.
    m_previousFindText = QString();
}

void FindTextHelper::invokeResultCallback(int requestId, int numberOfMatches)
{
    if (m_quickCallbacks.contains(requestId)) {
        QJSValue resultCallback = m_quickCallbacks.take(requestId);
        QJSValueList args;
        args.append(QJSValue(numberOfMatches));
        resultCallback.call(args);
    } else {
        m_widgetCallbacks.invoke(requestId, numberOfMatches > 0);
    }
}

} // namespace QtWebEngineCore
