// Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef RENDER_VIEW_CONTEXT_MENU_QT_H
#define RENDER_VIEW_CONTEXT_MENU_QT_H

#include "web_contents_adapter_client.h"

QT_FORWARD_DECLARE_CLASS(QWebEngineContextMenuRequest)

namespace QtWebEngineCore {

class Q_WEBENGINECORE_PRIVATE_EXPORT RenderViewContextMenuQt
{
public:
    enum ContextMenuItem {
        Back = 0,
        Forward,
        Reload,

        Cut,
        Copy,
        Paste,

        Undo,
        Redo,
        SelectAll,

        PasteAndMatchStyle,

        OpenLinkInNewWindow,
        OpenLinkInNewTab,
        CopyLinkToClipboard,
        DownloadLinkToDisk,

        CopyImageToClipboard,
        CopyImageUrlToClipboard,
        DownloadImageToDisk,

        CopyMediaUrlToClipboard,
        ToggleMediaControls,
        ToggleMediaLoop,
        DownloadMediaToDisk,

        InspectElement,
        ExitFullScreen,
        SavePage,
        ViewSource,

        SpellingSuggestions,

        Separator
    };

    static const QString getMenuItemName(RenderViewContextMenuQt::ContextMenuItem menuItem);

    RenderViewContextMenuQt(QWebEngineContextMenuRequest *data);
    void initMenu();

protected:
    virtual bool hasInspector() = 0;
    virtual bool isFullScreenMode() = 0;

    virtual void addMenuItem(ContextMenuItem menuItem) = 0;
    virtual bool isMenuItemEnabled(ContextMenuItem menuItem) = 0;

    QWebEngineContextMenuRequest *m_contextData;

private:
    void appendCanvasItems();
    void appendCopyItem();
    void appendEditableItems();
    void appendExitFullscreenItem();
    void appendDeveloperItems();
    void appendImageItems();
    void appendLinkItems();
    void appendMediaItems();
    void appendPageItems();
    void appendSpellingSuggestionItems();
    void appendSeparatorItem();
    bool canViewSource();
};

}

#endif // RENDER_VIEW_CONTEXT_MENU_QT_H
