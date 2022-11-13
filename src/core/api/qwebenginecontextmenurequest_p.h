// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINECONTEXTMENUREQUEST_P_H
#define QWEBENGINECONTEXTMENUREQUEST_P_H

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

#include "qtwebenginecoreglobal_p.h"
#include "qwebenginecontextmenurequest.h"
#include <QPoint>
#include <QUrl>

QT_BEGIN_NAMESPACE

class QWebEngineContextMenuRequestPrivate
{
public:
    bool m_accepted = false;
    bool m_hasImageContent = false;
    bool m_isEditable = false;
    bool m_isSpellCheckerEnabled = false;
    uint m_mediaType = 0;
    uint m_mediaFlags = 0;
    uint m_editFlags = 0;
    QPoint m_position;
    QUrl m_filteredLinkUrl;
    QUrl m_unfilteredLinkUrl;
    QUrl m_mediaUrl;
    QString m_altText;
    QString m_linkText;
    QString m_titleText;
    QString m_selectedText;
    QString m_suggestedFileName;
    QString m_misspelledWord;
    QStringList m_spellCheckerSuggestions;
    QUrl m_pageUrl;
    QUrl m_frameUrl;
    QtWebEngineCore::ReferrerPolicy m_referrerPolicy = QtWebEngineCore::ReferrerPolicy::Default;
    // Some likely candidates for future additions as we add support for the related actions:
    //    bool isImageBlocked;
    //    <enum tbd> mediaType;
    //    ...
};

QT_END_NAMESPACE

#endif
