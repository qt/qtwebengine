// Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef WEB_CONTENTS_ADAPTER_H
#define WEB_CONTENTS_ADAPTER_H

#include <QtCore/QSharedPointer>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtCore/QPointer>
#include <QtGui/qtgui-config.h>
#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QtWebEngineCore/qwebenginecontextmenurequest.h>
#include <QtWebEngineCore/qwebenginehttprequest.h>
#include <QtWebEngineCore/qwebengineframe.h>
#include <QtWebEngineCore/qwebenginepermission.h>

#include "web_contents_adapter_client.h"

#include <functional>
#include <memory>
#include <optional>

namespace blink {
namespace web_pref {
struct WebPreferences;
}
}

namespace base {
class Value;
}

namespace content {
class WebContents;
class SiteInstance;
class RenderFrameHost;
}

QT_BEGIN_NAMESPACE
class QAccessibleInterface;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QMimeData;
class QPageLayout;
class QPageRanges;
class QTemporaryDir;
class QWebChannel;
class QWebEngineUrlRequestInterceptor;
QT_END_NAMESPACE

namespace QtWebEngineCore {

class DevToolsFrontendQt;
class FindTextHelper;
class ProfileQt;
class WebEnginePageHost;
class WebChannelIPCTransportHost;

class Q_WEBENGINECORE_EXPORT WebContentsAdapter : public QEnableSharedFromThis<WebContentsAdapter> {
public:
    // Sentinel to indicate that a behavior should happen on the main frame
    static constexpr quint64 kUseMainFrameId = -2;
    // Sentinel to indicate a frame doesn't exist, for example with `findFrameByName`
    static constexpr quint64 kInvalidFrameId = -3;

    static QSharedPointer<WebContentsAdapter> createFromSerializedNavigationHistory(QDataStream &input, WebContentsAdapterClient *adapterClient);
    WebContentsAdapter();
    WebContentsAdapter(std::unique_ptr<content::WebContents> webContents);
    ~WebContentsAdapter();

    void setClient(WebContentsAdapterClient *adapterClient);

    bool isInitialized() const;

    // These and only these methods will initialize the WebContentsAdapter. All
    // other methods below will do nothing until one of these has been called.
    void loadDefault();
    void load(const QUrl &url);
    void load(const QWebEngineHttpRequest &request);
    void setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl);

    using LifecycleState = WebContentsAdapterClient::LifecycleState;
    LifecycleState lifecycleState() const;
    void setLifecycleState(LifecycleState state);
    LifecycleState recommendedState() const;

    bool isVisible() const;
    void setVisible(bool visible);

    bool canGoBack() const;
    bool canGoForward() const;
    bool canGoToOffset(int) const;
    void stop();
    void reload();
    void reloadAndBypassCache();
    void save(const QString &filePath = QString(), int savePageFormat = -1);
    QUrl activeUrl() const;
    QUrl requestedUrl() const;
    QString pageTitle() const;
    QString selectedText() const;
    QUrl iconUrl() const;
    QIcon icon() const;

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void pasteAndMatchStyle();
    void selectAll();
    void unselect();

    void navigateBack();
    void navigateForward();
    void navigateToIndex(int);
    void navigateToOffset(int);
    int navigationEntryCount();
    int currentNavigationEntryIndex();
    QUrl getNavigationEntryOriginalUrl(int index);
    QUrl getNavigationEntryUrl(int index);
    QString getNavigationEntryTitle(int index);
    QDateTime getNavigationEntryTimestamp(int index);
    QUrl getNavigationEntryIconUrl(int index);
    void clearNavigationHistory();
    void serializeNavigationHistory(QDataStream &output);
    void setZoomFactor(qreal);
    qreal currentZoomFactor() const;
    void runJavaScript(const QString &javaScript, quint32 worldId, quint64 frameId,
                       const std::function<void(const QVariant &)> &callback);
    void didRunJavaScript(quint64 requestId, const base::Value &result);
    void clearJavaScriptCallbacks();
    quint64 fetchDocumentMarkup();
    quint64 fetchDocumentInnerText();
    void updateWebPreferences(const blink::web_pref::WebPreferences &webPreferences);
    void download(const QUrl &url, const QString &suggestedFileName,
                  const QUrl &referrerUrl = QUrl(),
                  ReferrerPolicy referrerPolicy = ReferrerPolicy::Default);
    bool isAudioMuted() const;
    void setAudioMuted(bool mute);
    bool recentlyAudible() const;
    qint64 renderProcessPid() const;

    // Must match blink::WebMediaPlayerAction::Type.
    enum MediaPlayerAction {
        MediaPlayerNoAction,
        MediaPlayerPlay,
        MediaPlayerMute,
        MediaPlayerLoop,
        MediaPlayerControls,
        MediaPlayerTypeLast = MediaPlayerControls
    };
    void copyImageAt(const QPoint &location);
    void executeMediaPlayerActionAt(const QPoint &location, MediaPlayerAction action, bool enable);

    void inspectElementAt(const QPoint &location);
    bool hasInspector() const;
    bool isInspector() const;
    void setInspector(bool inspector);
    void exitFullScreen();
    void requestClose();
    void changedFullScreen();
    void openDevToolsFrontend(QSharedPointer<WebContentsAdapter> devtoolsFrontend);
    void closeDevToolsFrontend();
    void devToolsFrontendDestroyed(DevToolsFrontendQt *frontend);
    QString devToolsId();

    void setPermission(const QUrl &origin, QWebEnginePermission::PermissionType permissionType, QWebEnginePermission::State state);
    QWebEnginePermission::State getPermissionState(const QUrl &origin, QWebEnginePermission::PermissionType permissionType);

    void grantMediaAccessPermission(const QUrl &origin, WebContentsAdapterClient::MediaRequestFlags flags);
    void grantMouseLockPermission(const QUrl &origin, bool granted);
    void handlePendingMouseLockPermission();

    void setBackgroundColor(const QColor &color);
    QAccessibleInterface *browserAccessible();
    ProfileQt* profile();
    ProfileAdapter* profileAdapter();
#if QT_CONFIG(webengine_webchannel)
    QWebChannel *webChannel() const;
    void setWebChannel(QWebChannel *, uint worldId);
    WebChannelIPCTransportHost *webChannelTransport() { return m_webChannelTransport.get(); }
#endif
    FindTextHelper *findTextHelper();

    QPointF lastScrollOffset() const;
    QSizeF lastContentsSize() const;

#if QT_CONFIG(draganddrop)
    void startDragging(QObject *dragSource, const content::DropData &dropData,
                       Qt::DropActions allowedActions, const QPixmap &pixmap, const QPoint &offset);
    void enterDrag(QDragEnterEvent *e, const QPointF &screenPos);
    Qt::DropAction updateDragPosition(QDragMoveEvent *e, const QPointF &screenPos);
    void updateDragAction(int action, bool documentIsHandlingDrag);
    void endDragging(QDropEvent *e, const QPointF &screenPos);
    void leaveDrag();
#endif // QT_CONFIG(draganddrop)
    void printToPDF(const QPageLayout &, const QPageRanges &, const QString &, quint64 frameId);
    void printToPDFCallbackResult(std::function<void(QSharedPointer<QByteArray>)> &&,
                                  const QPageLayout &, const QPageRanges &, bool colorMode,
                                  bool useCustomMargins, quint64 frameId);
    void didPrintPage(quint64 requestId, QSharedPointer<QByteArray> result);

    void replaceMisspelling(const QString &word);
    void viewSource();
    bool canViewSource();
    void focusIfNecessary();
    bool isFindTextInProgress() const;
    bool hasFocusedFrame() const;
    void resetSelection();
    void resetTouchSelectionController();
    void changeTextDirection(bool leftToRight);

    quint64 mainFrameId() const;
    QString frameName(quint64 id) const;
    QString frameHtmlName(quint64 id) const;
    QList<quint64> frameChildren(quint64 id) const;
    QUrl frameUrl(quint64 id) const;
    QSizeF frameSize(quint64 id) const;
    std::optional<quint64> findFrameIdByName(const QString &name) const;
    bool hasFrame(quint64 id) const;

    // meant to be used within WebEngineCore only
    void initialize(content::SiteInstance *site);
    content::WebContents *webContents() const;
    content::WebContents *guestWebContents() const;
    WebContentsAdapterClient *adapterClient();
    void updateRecommendedState();
    void setRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor);
    QWebEngineUrlRequestInterceptor* requestInterceptor() const;

private:
    Q_DISABLE_COPY(WebContentsAdapter)
    void waitForUpdateDragActionCalled();
    bool handleDropDataFileContents(const content::DropData &dropData, QMimeData *mimeData);
    content::RenderFrameHost *renderFrameHostFromFrameId(quint64 frameId) const;

    void wasShown();
    void wasHidden();

    LifecycleState determineRecommendedState() const;

    void freeze();
    void unfreeze();
    void discard();
    void undiscard();

    void initializeRenderPrefs();

    ProfileAdapter *m_profileAdapter;
    std::unique_ptr<content::WebContents> m_webContents;
    std::unique_ptr<WebContentsDelegateQt> m_webContentsDelegate;
    std::unique_ptr<WebEnginePageHost> m_pageHost;
#if QT_CONFIG(webengine_webchannel)
    std::unique_ptr<WebChannelIPCTransportHost> m_webChannelTransport;
    QWebChannel *m_webChannel;
    unsigned int m_webChannelWorld;
#endif
    WebContentsAdapterClient *m_adapterClient;
    quint64 m_nextRequestId;
    QMap<QUrl, bool> m_pendingMouseLockPermissions;
    QMap<quint64, std::function<void(const QVariant &)>> m_javaScriptCallbacks;
    std::map<quint64, std::function<void(QSharedPointer<QByteArray>)>> m_printCallbacks;
    std::unique_ptr<content::DropData> m_currentDropData;
    uint m_currentDropAction;
    bool m_updateDragActionCalled;
    QPointF m_lastDragClientPos;
    QPointF m_lastDragScreenPos;
    std::unique_ptr<QTemporaryDir> m_dndTmpDir;
    DevToolsFrontendQt *m_devToolsFrontend;
    LifecycleState m_lifecycleState = LifecycleState::Active;
    LifecycleState m_recommendedState = LifecycleState::Active;
    bool m_inspector = false;
    bool m_documentIsHandlingDrag = false;
    QPointer<QWebEngineUrlRequestInterceptor> m_requestInterceptor;
};

} // namespace QtWebEngineCore

#endif // WEB_CONTENTS_ADAPTER_H
