// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINECONTEXTMENUREQUEST_H
#define QWEBENGINECONTEXTMENUREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qpoint.h>
#include <QtCore/qscopedpointer.h>

namespace extensions {
class MimeHandlerViewGuestDelegateQt;
}

namespace QtWebEngineCore {
class RenderViewContextMenuQt;
class WebContentsViewQt;

// Must match blink::WebReferrerPolicy
enum class ReferrerPolicy {
    Always,
    Default,
    NoReferrerWhenDowngrade,
    Never,
    Origin,
    OriginWhenCrossOrigin,
    NoReferrerWhenDowngradeOriginWhenCrossOrigin,
    SameOrigin,
    StrictOrigin,
    Last = StrictOrigin,
};
}

QT_BEGIN_NAMESPACE

class QWebEngineContextMenuRequestPrivate;
class Q_WEBENGINECORE_EXPORT QWebEngineContextMenuRequest : public QObject
{
    Q_OBJECT
public:
    // Must match blink::mojom::ContextMenuDataMediaType:
    enum MediaType {
        MediaTypeNone,
        MediaTypeImage,
        MediaTypeVideo,
        MediaTypeAudio,
        MediaTypeCanvas,
        MediaTypeFile,
        MediaTypePlugin
    };
    Q_ENUM(MediaType)

    // Must match blink::ContextMenuData::MediaFlags:
    enum MediaFlag {
        MediaInError = 0x1,
        MediaPaused = 0x2,
        MediaMuted = 0x4,
        MediaLoop = 0x8,
        MediaCanSave = 0x10,
        MediaHasAudio = 0x20,
        MediaCanToggleControls = 0x40,
        MediaControls = 0x80,
        MediaCanPrint = 0x100,
        MediaCanRotate = 0x200,
    };
    Q_DECLARE_FLAGS(MediaFlags, MediaFlag)
    Q_FLAG(MediaFlags)

    // Must match blink::ContextMenuDataEditFlags:
    enum EditFlag {
        CanUndo = 0x1,
        CanRedo = 0x2,
        CanCut = 0x4,
        CanCopy = 0x8,
        CanPaste = 0x10,
        CanDelete = 0x20,
        CanSelectAll = 0x40,
        CanTranslate = 0x80,
        CanEditRichly = 0x100,
    };
    Q_DECLARE_FLAGS(EditFlags, EditFlag)
    Q_FLAG(EditFlags)

    Q_PROPERTY(QPoint position READ position CONSTANT FINAL)
    Q_PROPERTY(QString selectedText READ selectedText CONSTANT FINAL)
    Q_PROPERTY(QString linkText READ linkText CONSTANT FINAL)
    Q_PROPERTY(QUrl linkUrl READ linkUrl CONSTANT FINAL)
    Q_PROPERTY(QUrl mediaUrl READ mediaUrl CONSTANT FINAL)
    Q_PROPERTY(MediaType mediaType READ mediaType CONSTANT FINAL)
    Q_PROPERTY(bool isContentEditable READ isContentEditable CONSTANT FINAL)
    Q_PROPERTY(QString misspelledWord READ misspelledWord CONSTANT FINAL)
    Q_PROPERTY(QStringList spellCheckerSuggestions READ spellCheckerSuggestions CONSTANT FINAL)
    Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted FINAL)
    Q_PROPERTY(MediaFlags mediaFlags READ mediaFlags CONSTANT FINAL REVISION(1, 1))
    Q_PROPERTY(EditFlags editFlags READ editFlags CONSTANT FINAL REVISION(1, 1))

    virtual ~QWebEngineContextMenuRequest();
    QPoint position() const;
    QString selectedText() const;
    QString linkText() const;
    QUrl linkUrl() const;
    QUrl mediaUrl() const;
    MediaType mediaType() const;
    bool isContentEditable() const;
    QString misspelledWord() const;
    QStringList spellCheckerSuggestions() const;
    bool isAccepted() const;
    void setAccepted(bool accepted);
    MediaFlags mediaFlags() const;
    EditFlags editFlags() const;

private:
    QUrl filteredLinkUrl() const;
    QString altText() const;
    QString titleText() const;
    QUrl referrerUrl() const;
    QtWebEngineCore::ReferrerPolicy referrerPolicy() const;
    bool hasImageContent() const;
    QString suggestedFileName() const;

private:
    QWebEngineContextMenuRequest(QWebEngineContextMenuRequestPrivate *d);
    QScopedPointer<QWebEngineContextMenuRequestPrivate> d;
    friend class QtWebEngineCore::WebContentsViewQt;
    friend class QtWebEngineCore::RenderViewContextMenuQt;
    friend class extensions::MimeHandlerViewGuestDelegateQt;
    friend class QQuickWebEngineViewPrivate;
    friend class QQuickWebEngineView;
    friend class QWebEnginePage;
};

QT_END_NAMESPACE

#endif // QWEBENGINECONTEXTMENUREQUEST_H
