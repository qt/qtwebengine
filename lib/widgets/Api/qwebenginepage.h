/*
    Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2007 Staikos Computing Services Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QWEBENGINEPAGE_H
#define QWEBENGINEPAGE_H

#include <QtWebEngineWidgets/qtwebenginewidgetsglobal.h>
#include <QtWebEngineWidgets/qwebenginesettings.h>

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE
class QUndoStack;
class QMenu;
class QPrinter;
QT_END_NAMESPACE

// FIXME: Just forward-declare the to-be-removed frame and element classes for now.
// Referencing calls should be ported to be page-friendly or removed individually.
class QWebEngineFrame;
class QWebEngineElement;
class QWebEngineElementCollection;

class QWebEngineHistory;
class QWebEngineHistoryItem;
class QWebEnginePagePrivate;
class QWebEnginePluginFactory;
class QWebEngineSecurityOrigin;
class QtViewportAttributesPrivate;
class QWebEngineHitTestResultPrivate;

class QWEBENGINEWIDGETS_EXPORT QWebEngineHitTestResult {
public:
    QWebEngineHitTestResult();
    QWebEngineHitTestResult(const QWebEngineHitTestResult &other);
    QWebEngineHitTestResult &operator=(const QWebEngineHitTestResult &other);
    ~QWebEngineHitTestResult();

    bool isNull() const;

    QPoint pos() const;
    QRect boundingRect() const;
    QWebEngineElement enclosingBlockElement() const;
    QString title() const;

    QString linkText() const;
    QUrl linkUrl() const;
    QUrl linkTitle() const;
    QWebEngineFrame *linkTargetFrame() const;
    QWebEngineElement linkElement() const;

    QString alternateText() const; // for img, area, input and applet

    QUrl imageUrl() const;
    QPixmap pixmap() const;

    bool isContentEditable() const;
    bool isContentSelected() const;

    QWebEngineElement element() const;

    QWebEngineFrame *frame() const;
};

class QWEBENGINEWIDGETS_EXPORT QWebEnginePage : public QObject {
    Q_OBJECT
// Hack to avoid undefined symbols with properties until we have them implemented.
#ifndef Q_MOC_RUN
    Q_PROPERTY(bool modified READ isModified)
    Q_PROPERTY(QString selectedText READ selectedText)
    Q_PROPERTY(QString selectedHtml READ selectedHtml)
    Q_PROPERTY(bool hasSelection READ hasSelection)
    Q_PROPERTY(QSize viewportSize READ viewportSize WRITE setViewportSize)
    Q_PROPERTY(QSize preferredContentsSize READ preferredContentsSize WRITE setPreferredContentsSize)
    Q_PROPERTY(bool forwardUnsupportedContent READ forwardUnsupportedContent WRITE setForwardUnsupportedContent)
    Q_PROPERTY(LinkDelegationPolicy linkDelegationPolicy READ linkDelegationPolicy WRITE setLinkDelegationPolicy)
    Q_PROPERTY(QPalette palette READ palette WRITE setPalette)
    Q_PROPERTY(bool contentEditable READ isContentEditable WRITE setContentEditable)
    Q_ENUMS(LinkDelegationPolicy NavigationType WebAction)

    // Ex-QWebFrame properties
    Q_PROPERTY(qreal textSizeMultiplier READ textSizeMultiplier WRITE setTextSizeMultiplier DESIGNABLE false)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(QUrl requestedUrl READ requestedUrl)
    Q_PROPERTY(QUrl baseUrl READ baseUrl)
    Q_PROPERTY(QIcon icon READ icon)
    Q_PROPERTY(QSize contentsSize READ contentsSize)
    Q_PROPERTY(QPoint scrollPosition READ scrollPosition WRITE setScrollPosition)
    Q_PROPERTY(bool focus READ hasFocus)
#endif

public:
    enum NavigationType {
        NavigationTypeLinkClicked,
        NavigationTypeFormSubmitted,
        NavigationTypeBackOrForward,
        NavigationTypeReload,
        NavigationTypeFormResubmitted,
        NavigationTypeOther
    };

    enum WebAction {
        NoWebAction = - 1,

        OpenLink,

        OpenLinkInNewWindow,
        OpenFrameInNewWindow,

        DownloadLinkToDisk,
        CopyLinkToClipboard,

        OpenImageInNewWindow,
        DownloadImageToDisk,
        CopyImageToClipboard,

        Back,
        Forward,
        Stop,
        Reload,

        Cut,
        Copy,
        Paste,

        Undo,
        Redo,
        MoveToNextChar,
        MoveToPreviousChar,
        MoveToNextWord,
        MoveToPreviousWord,
        MoveToNextLine,
        MoveToPreviousLine,
        MoveToStartOfLine,
        MoveToEndOfLine,
        MoveToStartOfBlock,
        MoveToEndOfBlock,
        MoveToStartOfDocument,
        MoveToEndOfDocument,
        SelectNextChar,
        SelectPreviousChar,
        SelectNextWord,
        SelectPreviousWord,
        SelectNextLine,
        SelectPreviousLine,
        SelectStartOfLine,
        SelectEndOfLine,
        SelectStartOfBlock,
        SelectEndOfBlock,
        SelectStartOfDocument,
        SelectEndOfDocument,
        DeleteStartOfWord,
        DeleteEndOfWord,

        SetTextDirectionDefault,
        SetTextDirectionLeftToRight,
        SetTextDirectionRightToLeft,

        ToggleBold,
        ToggleItalic,
        ToggleUnderline,

        InspectElement,

        InsertParagraphSeparator,
        InsertLineSeparator,

        SelectAll,
        ReloadAndBypassCache,

        PasteAndMatchStyle,
        RemoveFormat,

        ToggleStrikethrough,
        ToggleSubscript,
        ToggleSuperscript,
        InsertUnorderedList,
        InsertOrderedList,
        Indent,
        Outdent,

        AlignCenter,
        AlignJustified,
        AlignLeft,
        AlignRight,

        StopScheduledPageRefresh,

        CopyImageUrlToClipboard,

        OpenLinkInThisWindow,

        WebActionCount
    };

    enum FindFlag {
        FindBackward = 1,
        FindCaseSensitively = 2,
        FindWrapsAroundDocument = 4,
        HighlightAllOccurrences = 8
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    enum LinkDelegationPolicy {
        DontDelegateLinks,
        DelegateExternalLinks,
        DelegateAllLinks
    };

    enum WebWindowType {
        WebBrowserWindow,
        WebModalDialog
    };

    enum PermissionPolicy {
        PermissionUnknown,
        PermissionGrantedByUser,
        PermissionDeniedByUser
    };

    enum Feature {
        Notifications,
        Geolocation
    };

    // Ex-QWebFrame enum
    enum ValueOwnership {
        QtOwnership,
        ScriptOwnership,
        AutoOwnership
    };

    class QWEBENGINEWIDGETS_EXPORT ViewportAttributes {
    public:
        ViewportAttributes();
        ViewportAttributes(const QWebEnginePage::ViewportAttributes& other);

        ~ViewportAttributes();

        QWebEnginePage::ViewportAttributes& operator=(const QWebEnginePage::ViewportAttributes& other);

        inline qreal initialScaleFactor() const { return m_initialScaleFactor; }
        inline qreal minimumScaleFactor() const { return m_minimumScaleFactor; }
        inline qreal maximumScaleFactor() const { return m_maximumScaleFactor; }
        inline qreal devicePixelRatio() const { return m_devicePixelRatio; }
        inline bool isUserScalable() const { return m_isUserScalable; }
        inline bool isValid() const { return m_isValid; }
        inline QSizeF size() const { return m_size; }

    private:
        QSharedDataPointer<QtViewportAttributesPrivate> d;
        qreal m_initialScaleFactor;
        qreal m_minimumScaleFactor;
        qreal m_maximumScaleFactor;
        qreal m_devicePixelRatio;
        bool m_isUserScalable;
        bool m_isValid;
        QSizeF m_size;

        friend class QWebEnginePage;
    };


    explicit QWebEnginePage(QObject *parent = 0);
    ~QWebEnginePage();

    QWebEngineFrame *mainFrame() const;
    QWebEngineFrame *currentFrame() const;
    QWebEngineFrame* frameAt(const QPoint& pos) const;

    QWebEngineHistory *history() const;
    QWebEngineSettings *settings() const;

    void setView(QWidget *view);
    QWidget *view() const;

    bool isModified() const;
#ifndef QT_NO_UNDOSTACK
    QUndoStack *undoStack() const;
#endif

    void setNetworkAccessManager(QNetworkAccessManager *manager);
    QNetworkAccessManager *networkAccessManager() const;

    void setPluginFactory(QWebEnginePluginFactory *factory);
    QWebEnginePluginFactory *pluginFactory() const;

    quint64 totalBytes() const;
    quint64 bytesReceived() const;

    bool hasSelection() const;
    QString selectedText() const;
    QString selectedHtml() const;

#ifndef QT_NO_ACTION
    QAction *action(WebAction action) const;
#endif
    virtual void triggerAction(WebAction action, bool checked = false);

    QSize viewportSize() const;
    void setViewportSize(const QSize &size) const;
    ViewportAttributes viewportAttributesForSize(const QSize& availableSize) const;

    QSize preferredContentsSize() const;
    void setPreferredContentsSize(const QSize &size) const;
    void setActualVisibleContentRect(const QRect& rect) const;

    bool focusNextPrevChild(bool next);

    QVariant inputMethodQuery(Qt::InputMethodQuery property) const;

    bool findText(const QString &subString, FindFlags options = 0);

    void setForwardUnsupportedContent(bool forward);
    bool forwardUnsupportedContent() const;

    void setLinkDelegationPolicy(LinkDelegationPolicy policy);
    LinkDelegationPolicy linkDelegationPolicy() const;

    void setPalette(const QPalette &palette);
    QPalette palette() const;

    void setContentEditable(bool editable);
    bool isContentEditable() const;

#ifndef QT_NO_CONTEXTMENU
    bool swallowContextMenuEvent(QContextMenuEvent *event);
#endif
    void updatePositionDependentActions(const QPoint &pos);

    QMenu *createStandardContextMenu();

    void setFeaturePermission(QWebEngineFrame* frame, Feature feature, PermissionPolicy policy);

    QStringList supportedContentTypes() const;
    bool supportsContentType(const QString& mimeType) const;

    enum Extension {
        ChooseMultipleFilesExtension,
        ErrorPageExtension
    };
    class ExtensionOption
    {};
    class ExtensionReturn
    {};

    class ChooseMultipleFilesExtensionOption : public ExtensionOption {
    public:
        QWebEngineFrame *parentFrame;
        QStringList suggestedFileNames;
    };

    class ChooseMultipleFilesExtensionReturn : public ExtensionReturn {
    public:
        QStringList fileNames;
    };

    enum ErrorDomain { QtNetwork, Http, WebKit };
    class ErrorPageExtensionOption : public ExtensionOption {
    public:
        QUrl url;
        QWebEngineFrame* frame;
        ErrorDomain domain;
        int error;
        QString errorString;
    };

    class ErrorPageExtensionReturn : public ExtensionReturn {
    public:
        ErrorPageExtensionReturn() : contentType(QLatin1String("text/html")), encoding(QLatin1String("utf-8")) {};
        QString contentType;
        QString encoding;
        QUrl baseUrl;
        QByteArray content;
    };


    virtual bool extension(Extension extension, const ExtensionOption *option = 0, ExtensionReturn *output = 0) { Q_UNUSED(extension); Q_UNUSED(option); Q_UNUSED(output); Q_UNREACHABLE(); return false; }
    virtual bool supportsExtension(Extension extension) const { Q_UNUSED(extension); Q_UNREACHABLE(); return false; }

    virtual bool shouldInterruptJavaScript() { Q_UNREACHABLE(); return false; }

    // Ex-QWebFrame methods
    void load(const QUrl &url);
    void load(const QNetworkRequest &request, QNetworkAccessManager::Operation operation = QNetworkAccessManager::GetOperation, const QByteArray &body = QByteArray());
    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    void setContent(const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl());

    void addToJavaScriptWindowObject(const QString &name, QObject *object, ValueOwnership ownership = QtOwnership);
    QString toHtml() const;
    QString toPlainText() const;

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;
    QUrl requestedUrl() const;
    QUrl baseUrl() const;
    QIcon icon() const;
    QMultiMap<QString, QString> metaData() const;

    QString frameName() const;

    QWebEngineFrame *parentFrame() const;
    QList<QWebEngineFrame*> childFrames() const;

    Qt::ScrollBarPolicy scrollBarPolicy(Qt::Orientation orientation) const;
    void setScrollBarPolicy(Qt::Orientation orientation, Qt::ScrollBarPolicy policy);

    void setScrollBarValue(Qt::Orientation orientation, int value);
    int scrollBarValue(Qt::Orientation orientation) const;
    int scrollBarMinimum(Qt::Orientation orientation) const;
    int scrollBarMaximum(Qt::Orientation orientation) const;
    QRect scrollBarGeometry(Qt::Orientation orientation) const;

    void scroll(int, int);
    QPoint scrollPosition() const;
    void setScrollPosition(const QPoint &pos);

    void scrollToAnchor(const QString& anchor);

    enum RenderLayer {
        ContentsLayer = 0x10,
        ScrollBarLayer = 0x20,
        PanIconLayer = 0x40,

        AllLayers = 0xff
    };
    Q_DECLARE_FLAGS(RenderLayers, RenderLayer)

    void render(QPainter*, const QRegion& clip = QRegion());
    void render(QPainter*, RenderLayers layer, const QRegion& clip = QRegion());

    void setTextSizeMultiplier(qreal factor);
    qreal textSizeMultiplier() const;

    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);

    bool hasFocus() const;
    void setFocus();

    QPoint pos() const;
    QRect geometry() const;
    QSize contentsSize() const;

    QWebEngineElement documentElement() const;
    QWebEngineElementCollection findAllElements(const QString &selectorQuery) const;
    QWebEngineElement findFirstElement(const QString &selectorQuery) const;

    QWebEngineHitTestResult hitTestContent(const QPoint &pos) const;

    QWebEngineSecurityOrigin securityOrigin() const;

public Q_SLOTS:
    // Ex-QWebFrame slots
    QVariant evaluateJavaScript(const QString& scriptSource) { Q_UNUSED(scriptSource); Q_UNREACHABLE(); return QVariant(); };
#ifndef QT_NO_PRINTER
    void print(QPrinter *printer) const { Q_UNUSED(printer); Q_UNREACHABLE(); };
#endif


Q_SIGNALS:
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool ok);

    void linkHovered(const QString &link, const QString &title, const QString &textContent);
    void statusBarMessage(const QString& text);
    void selectionChanged();
    void frameCreated(QWebEngineFrame *frame);
    void geometryChangeRequested(const QRect& geom);
    void repaintRequested(const QRect& dirtyRect);
    void scrollRequested(int dx, int dy, const QRect& scrollViewRect);
    void windowCloseRequested();
    void printRequested(QWebEngineFrame *frame);
    void linkClicked(const QUrl &url);

    void toolBarVisibilityChangeRequested(bool visible);
    void statusBarVisibilityChangeRequested(bool visible);
    void menuBarVisibilityChangeRequested(bool visible);

    void unsupportedContent(QNetworkReply *reply);
    void downloadRequested(const QNetworkRequest &request);

    void microFocusChanged();
    void contentsChanged();
    void databaseQuotaExceeded(QWebEngineFrame* frame, QString databaseName);
    void applicationCacheQuotaExceeded(QWebEngineSecurityOrigin* origin, quint64 defaultOriginQuota, quint64 totalSpaceNeeded);

    void saveFrameStateRequested(QWebEngineFrame* frame, QWebEngineHistoryItem* item);
    void restoreFrameStateRequested(QWebEngineFrame* frame);

    void viewportChangeRequested();

    void featurePermissionRequested(QWebEngineFrame* frame, QWebEnginePage::Feature feature);
    void featurePermissionRequestCanceled(QWebEngineFrame* frame, QWebEnginePage::Feature feature);

    // Ex-QWebFrame signals
    void javaScriptWindowObjectCleared();

    void provisionalLoad();
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);

    void initialLayoutCompleted();

    void iconChanged();

    void contentsSizeChanged(const QSize &size);

    void pageChanged();

protected:
    virtual QWebEnginePage *createWindow(WebWindowType type);
    virtual QObject *createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues) { Q_UNUSED(classid); Q_UNUSED(url); Q_UNUSED(paramNames); Q_UNUSED(paramValues); Q_UNREACHABLE(); return 0; }

    virtual bool acceptNavigationRequest(QWebEngineFrame *frame, const QNetworkRequest &request, NavigationType type) { Q_UNUSED(frame); Q_UNUSED(request); Q_UNUSED(type); Q_UNREACHABLE(); return false; }
    virtual QString chooseFile(QWebEngineFrame *originatingFrame, const QString& oldFile) { Q_UNUSED(originatingFrame); Q_UNUSED(oldFile); Q_UNREACHABLE(); return QString(); }
    virtual void javaScriptAlert(QWebEngineFrame *originatingFrame, const QString& msg);
    virtual bool javaScriptConfirm(QWebEngineFrame *originatingFrame, const QString& msg);
    virtual bool javaScriptPrompt(QWebEngineFrame *originatingFrame, const QString& msg, const QString& defaultValue, QString* result);
    virtual void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID) { Q_UNUSED(message); Q_UNUSED(lineNumber); Q_UNUSED(sourceID); Q_UNREACHABLE(); }

    virtual QString userAgentForUrl(const QUrl& url) const { Q_UNUSED(url); Q_UNREACHABLE(); return QString(); }

private:
    Q_DECLARE_PRIVATE(QWebEnginePage);
#ifndef QT_NO_ACTION
    Q_PRIVATE_SLOT(d_func(), void _q_webActionTriggered(bool checked))
#endif

    friend class QWebEngineView;
    friend class QWebEngineViewPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWebEnginePage::FindFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(QWebEnginePage::RenderLayers);

#endif // QWEBENGINEPAGE_H
