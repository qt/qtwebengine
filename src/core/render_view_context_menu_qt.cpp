// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QCoreApplication>
#include "render_view_context_menu_qt.h"
#include "qwebenginecontextmenurequest.h"

namespace QtWebEngineCore {

    const QString RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem menuItem) {
        Q_ASSERT(menuItem <= ContextMenuItem::ViewSource);
        static const char *names[ContextMenuItem::ViewSource + 1] = {
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Back"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Forward"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Reload"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Cut"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Copy"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Paste"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Undo"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Redo"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Select all"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Paste and match style"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Open link in new window"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Open link in new tab"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Copy link address"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Save link"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Copy image"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Copy image address"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Save image"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Copy media address"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Show controls"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Loop"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Save media"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Inspect"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Exit full screen"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Save page"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "View page source")
        };
        return QCoreApplication::translate("RenderViewContextMenuQt", qUtf8Printable(names[menuItem]));
    }

    RenderViewContextMenuQt::RenderViewContextMenuQt(QWebEngineContextMenuRequest *request)
        : m_contextData(request)
    {
    }

    void RenderViewContextMenuQt::initMenu()
    {
        if (isFullScreenMode()) {
            appendExitFullscreenItem();
            appendSeparatorItem();
        }

        if (m_contextData->isContentEditable()
            && !m_contextData->spellCheckerSuggestions().isEmpty()) {
            appendSpellingSuggestionItems();
            appendSeparatorItem();
        }

        if (m_contextData->linkText().isEmpty() && !m_contextData->filteredLinkUrl().isValid()
            && !m_contextData->mediaUrl().isValid()) {
            if (m_contextData->isContentEditable())
                appendEditableItems();
            else if (!m_contextData->selectedText().isEmpty())
                appendCopyItem();
            else
                appendPageItems();
        } else {
            appendPageItems();
        }

        if (m_contextData->filteredLinkUrl().isValid()
            || !m_contextData->linkUrl().isEmpty()
            || !m_contextData->filteredLinkUrl().isEmpty())
            appendLinkItems();

        if (m_contextData->mediaUrl().isValid()) {
            switch (m_contextData->mediaType()) {
            case QWebEngineContextMenuRequest::MediaTypeImage:
                appendSeparatorItem();
                appendImageItems();
                break;
            case QWebEngineContextMenuRequest::MediaTypeCanvas:
                Q_UNREACHABLE();    // mediaUrl is invalid for canvases
                break;
            case QWebEngineContextMenuRequest::MediaTypeAudio:
            case QWebEngineContextMenuRequest::MediaTypeVideo:
                appendSeparatorItem();
                appendMediaItems();
                break;
            default:
                break;
            }
        } else if (m_contextData->mediaType() == QWebEngineContextMenuRequest::MediaTypeCanvas) {
            appendSeparatorItem();
            appendCanvasItems();
        }

        if (canViewSource() || hasInspector()) {
            appendSeparatorItem();
            appendDeveloperItems();
        }
    }

    void RenderViewContextMenuQt::appendCanvasItems()
    {
        addMenuItem(RenderViewContextMenuQt::DownloadImageToDisk);
        addMenuItem(RenderViewContextMenuQt::CopyImageToClipboard);
    }

    void RenderViewContextMenuQt::appendCopyItem()
    {
        addMenuItem(RenderViewContextMenuQt::Copy);
    }

    void RenderViewContextMenuQt::appendDeveloperItems()
    {
        if (canViewSource())
            addMenuItem(RenderViewContextMenuQt::ViewSource);
        if (hasInspector())
            addMenuItem(RenderViewContextMenuQt::InspectElement);
    }

    void RenderViewContextMenuQt::appendEditableItems()
    {
        addMenuItem(RenderViewContextMenuQt::Undo);
        addMenuItem(RenderViewContextMenuQt::Redo);
        appendSeparatorItem();
        addMenuItem(RenderViewContextMenuQt::Cut);
        addMenuItem(RenderViewContextMenuQt::Copy);
        addMenuItem(RenderViewContextMenuQt::Paste);
        if (m_contextData->misspelledWord().isEmpty()) {
            addMenuItem(RenderViewContextMenuQt::PasteAndMatchStyle);
            addMenuItem(RenderViewContextMenuQt::SelectAll);
        }
    }

    void RenderViewContextMenuQt::appendExitFullscreenItem()
    {
        addMenuItem(RenderViewContextMenuQt::ExitFullScreen);
    }

    void RenderViewContextMenuQt::appendImageItems()
    {
        addMenuItem(RenderViewContextMenuQt::DownloadImageToDisk);
        addMenuItem(RenderViewContextMenuQt::CopyImageToClipboard);
        addMenuItem(RenderViewContextMenuQt::CopyImageUrlToClipboard);
    }

    void RenderViewContextMenuQt::appendLinkItems()
    {
        addMenuItem(RenderViewContextMenuQt::OpenLinkInNewTab);
        addMenuItem(RenderViewContextMenuQt::OpenLinkInNewWindow);
        appendSeparatorItem();
        addMenuItem(RenderViewContextMenuQt::DownloadLinkToDisk);
        addMenuItem(RenderViewContextMenuQt::CopyLinkToClipboard);
    }

    void RenderViewContextMenuQt::appendMediaItems()
    {
        addMenuItem(RenderViewContextMenuQt::ToggleMediaLoop);
        if (m_contextData->mediaFlags() & QWebEngineContextMenuRequest::MediaCanToggleControls)
            addMenuItem(RenderViewContextMenuQt::ToggleMediaControls);
        addMenuItem(RenderViewContextMenuQt::DownloadMediaToDisk);
        addMenuItem(RenderViewContextMenuQt::CopyMediaUrlToClipboard);
    }

    void RenderViewContextMenuQt::appendPageItems()
    {
        addMenuItem(RenderViewContextMenuQt::Back);
        addMenuItem(RenderViewContextMenuQt::Forward);
        addMenuItem(RenderViewContextMenuQt::Reload);
        appendSeparatorItem();
        addMenuItem(RenderViewContextMenuQt::SavePage);
    }

    void RenderViewContextMenuQt::appendSpellingSuggestionItems()
    {
        addMenuItem(RenderViewContextMenuQt::SpellingSuggestions);
    }

    void RenderViewContextMenuQt::appendSeparatorItem()
    {
        addMenuItem(RenderViewContextMenuQt::Separator);
    }

    bool RenderViewContextMenuQt::canViewSource()
    {
        return m_contextData->linkText().isEmpty() && !m_contextData->filteredLinkUrl().isValid()
                && !m_contextData->mediaUrl().isValid() && !m_contextData->isContentEditable()
                && m_contextData->selectedText().isEmpty();
    }
}
