// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

void FindTextHelper::startFinding(const QString &findText, bool caseSensitively, bool findBackward, const std::function<void(const QWebEngineFindTextResult &)> &resultCallback)
{
    if (findText.isEmpty()) {
        stopFinding();
        m_viewClient->findTextFinished(QWebEngineFindTextResult());
        if (resultCallback)
            resultCallback(QWebEngineFindTextResult());
        return;
    }

    startFinding(findText, caseSensitively, findBackward);
    m_widgetCallbacks.insert(m_currentFindRequestId, resultCallback);
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
    Q_ASSERT(!findText.isEmpty());

    const bool findNext = !m_previousFindText.isEmpty() && findText == m_previousFindText;
    if (isFindTextInProgress()) {
        // There are cases where the render process will overwrite a previous request
        // with the new search and we'll have a dangling callback, leaving the application
        // waiting for it forever.
        // Assume that any unfinished find has been unsuccessful when a new one is started
        // to cover that case.
        m_lastCompletedFindRequestId = m_currentFindRequestId;
        if (!findNext)
            m_webContents->StopFinding(content::STOP_FIND_ACTION_KEEP_SELECTION);
        m_viewClient->findTextFinished(QWebEngineFindTextResult());
        invokeResultCallback(m_currentFindRequestId, 0, 0);
    }

    blink::mojom::FindOptionsPtr options = blink::mojom::FindOptions::New();
    options->forward = !findBackward;
    options->match_case = caseSensitively;
    options->new_session = !findNext;
    m_previousFindText = findText;

    m_currentFindRequestId = m_findRequestIdCounter++;
    m_webContents->Find(m_currentFindRequestId, toString16(findText), std::move(options), /*skip_delay=*/true);
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
    invokeResultCallback(requestId, numberOfMatches, activeMatch);
}

void FindTextHelper::handleLoadCommitted()
{
    // Make sure that we don't set the findNext WebFindOptions on a new frame.
    m_previousFindText = QString();
}

void FindTextHelper::invokeResultCallback(int requestId, int numberOfMatches, int activeMatch)
{
    if (m_quickCallbacks.contains(requestId)) {
        QJSValue resultCallback = m_quickCallbacks.take(requestId);
        QJSValueList args;
        args.append(QJSValue(numberOfMatches));
        resultCallback.call(args);
    } else if (auto func = m_widgetCallbacks.take(requestId)) {
        func(QWebEngineFindTextResult(numberOfMatches, activeMatch));
    }
}

} // namespace QtWebEngineCore
