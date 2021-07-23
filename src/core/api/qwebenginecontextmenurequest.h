/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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
