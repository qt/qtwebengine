/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browser_accessibility_qt.h"

#include "content/common/accessibility_node_data.h"
#include "third_party/WebKit/public/web/WebAXEnums.h"
//#include "third_party/WebKit/public/web/WebAccessibilityRole.h"

#include "type_conversion.h"
#include "browser_accessibility_manager_qt.h"

using namespace WebKit;

namespace content {

bool BrowserAccessibilityQt::isValid() const
{
    return true;
}

QObject *BrowserAccessibilityQt::object() const
{
    return 0;
}

QAccessibleInterface *BrowserAccessibilityQt::childAt(int x, int y) const
{
    return 0;
}

QAccessibleInterface *BrowserAccessibilityQt::parent() const
{
    BrowserAccessibility *p = BrowserAccessibility::parent();
    if (p)
        return static_cast<BrowserAccessibilityQt*>(p);
    return static_cast<BrowserAccessibilityManagerQt*>(manager())->rootParentAccessible();
}

QAccessibleInterface *BrowserAccessibilityQt::child(int index) const
{
    return static_cast<BrowserAccessibilityQt*>(GetChild(index));
}

int BrowserAccessibilityQt::childCount() const
{
    return child_count();
}

int BrowserAccessibilityQt::indexOfChild(const QAccessibleInterface *iface) const
{

    const BrowserAccessibilityQt *child = static_cast<const BrowserAccessibilityQt*>(iface);
    return child->index_in_parent();
}

QString BrowserAccessibilityQt::text(QAccessible::Text t) const
{
    std::string name_str = name();
    // FIXME
//    if (name_str.empty()) {
//        int title_elem_id;
//        if (GetIntAttribute(ATTR_TITLE_UI_ELEMENT,
//                                    &title_elem_id)) {
//            BrowserAccessibility* title_elem =
//                    manager_->GetFromRendererID(title_elem_id);
//            if (title_elem)
//                name_str = title_elem->GetTextRecursive();
//        }
//    }
    return toQt(name_str);
}

void BrowserAccessibilityQt::setText(QAccessible::Text t, const QString &text)
{
}

QRect BrowserAccessibilityQt::rect() const
{
    gfx::Rect bounds = GetGlobalBoundsRect();
    return QRect(bounds.x(), bounds.y(), bounds.width(), bounds.height());
}

QAccessible::Role BrowserAccessibilityQt::role() const
{
//    qDebug() << "Got role: " << BrowserAccessibility::role();

    switch (BrowserAccessibility::role()) {
    case WebAXRoleUnknown:
        return QAccessible::NoRole;

    // Used by Chromium to distinguish between the root of the tree
    // for this page, and a web area for a frame within this page.
    case WebAXRoleWebArea:
        return QAccessible::Document;

    // These roles all directly correspond to WebKit accessibility roles,
    // keep these alphabetical.
    case WebAXRoleAlert:
        return QAccessible::AlertMessage;
//    case WebKit::WebAccessibilityRoleAnnotation:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::WebAccessibilityRoleApplication:
//        return QAccessible::Application;
//    case WebKit::WebAccessibilityRoleDocumentArticle:
//        return QAccessible::Document; // FIXME
//    case WebKit::WebAccessibilityRoleBrowser:
//        return QAccessible::Document; // FIXME
//    case WebKit::WebAccessibilityRoleBusyIndicator:
//        return QAccessible::Animation; // FIXME
//    case WebKit::WebAccessibilityRoleButton:
//        return QAccessible::Button;
//    case WebKit::WebAccessibilityRoleCanvas:
//        return QAccessible::Canvas;
//    case WebKit::ROLE_CELL:
//        return QAccessible::Cell;
//    case WebKit::ROLE_CHECKBOX:
//        return QAccessible::CheckBox;
//    case WebKit::ROLE_COLOR_WELL:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_COLUMN:
//        return QAccessible::Column;
//    case WebKit::ROLE_COLUMN_HEADER:
//        return QAccessible::ColumnHeader;
//    case WebKit::ROLE_COMBO_BOX:
//        return QAccessible::ComboBox;
//    case WebKit::ROLE_DEFINITION:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_DESCRIPTION_LIST_DETAIL:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_DESCRIPTION_LIST_TERM:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_DIALOG:
//        return QAccessible::Dialog;
//    case WebKit::ROLE_DIRECTORY:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_DISCLOSURE_TRIANGLE:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_DIV:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_DOCUMENT:
//        return QAccessible::Document;
//    case WebKit::ROLE_DRAWER:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_EDITABLE_TEXT:
//        return QAccessible::EditableText;
//    case WebKit::ROLE_FOOTER:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_FORM:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_GRID:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_GROUP:
//        return QAccessible::Grouping;
//    case WebKit::ROLE_GROW_AREA:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_HEADING:
//        return QAccessible::StaticText; // FIXME
//    case WebKit::ROLE_HELP_TAG:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_HORIZONTAL_RULE:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_IGNORED:
//        return QAccessible::NoRole;
//    case WebKit::ROLE_IMAGE:
//        return QAccessible::Graphic;
//    case WebKit::ROLE_IMAGE_MAP:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_IMAGE_MAP_LINK:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_INCREMENTOR:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LABEL:
//        return QAccessible::StaticText;
//    case WebKit::ROLE_LANDMARK_APPLICATION:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LANDMARK_BANNER:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LANDMARK_COMPLEMENTARY:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LANDMARK_CONTENTINFO:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LANDMARK_MAIN:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LANDMARK_NAVIGATION:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LANDMARK_SEARCH:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LINK:
//        return QAccessible::Link;
//    case WebKit::ROLE_LIST:
//        return QAccessible::List;
//    case WebKit::ROLE_LISTBOX:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LISTBOX_OPTION:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LIST_ITEM:
//        return QAccessible::ListItem;
//    case WebKit::ROLE_LIST_MARKER:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_LOG:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_MARQUEE:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_MATH:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_MATTE:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_MENU:
//        return QAccessible::PopupMenu;
//    case WebKit::ROLE_MENU_BAR:
//        return QAccessible::MenuBar;
//    case WebKit::ROLE_MENU_ITEM:
//        return QAccessible::MenuItem;
//    case WebKit::ROLE_MENU_BUTTON:
//        return QAccessible::MenuItem;
//    case WebKit::ROLE_MENU_LIST_OPTION:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_MENU_LIST_POPUP:
//        return QAccessible::PopupMenu;
//    case WebKit::ROLE_NOTE:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_OUTLINE:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_PARAGRAPH:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_POPUP_BUTTON:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_PRESENTATIONAL:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_PROGRESS_INDICATOR:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_RADIO_BUTTON:
//        return QAccessible::RadioButton;
//    case WebKit::ROLE_RADIO_GROUP:
//        return QAccessible::RadioButton;
//    case WebKit::ROLE_REGION:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_ROW:
//        return QAccessible::Row;
//    case WebKit::ROLE_ROW_HEADER:
//        return QAccessible::RowHeader;
//    case WebKit::ROLE_RULER:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_RULER_MARKER:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_SCROLLAREA:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_SCROLLBAR:
//        return QAccessible::ScrollBar;
//    case WebKit::ROLE_SHEET:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_SLIDER:
//        return QAccessible::Slider;
//    case WebKit::ROLE_SLIDER_THUMB:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_SPIN_BUTTON:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_SPIN_BUTTON_PART:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_SPLITTER:
//        return QAccessible::Splitter;
//    case WebKit::ROLE_SPLIT_GROUP:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_STATIC_TEXT:
//        return QAccessible::StaticText;
//    case WebKit::ROLE_STATUS:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_SVG_ROOT:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_SYSTEM_WIDE:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_TAB:
//        return QAccessible::PageTab;
//    case WebKit::ROLE_TABLE:
//        return QAccessible::Table;
//    case WebKit::ROLE_TABLE_HEADER_CONTAINER:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_TAB_GROUP_UNUSED:  // WebKit doesn't use (uses ROLE_TAB_LIST)
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_TAB_LIST:
//        return QAccessible::PageTabList;
//    case WebKit::ROLE_TAB_PANEL:
//        return QAccessible::PageTab;
//    case WebKit::ROLE_TEXTAREA:
//        return QAccessible::EditableText;
//    case WebKit::ROLE_TEXT_FIELD:
//        return QAccessible::EditableText;
//    case WebKit::ROLE_TIMER:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_TOGGLE_BUTTON:
//        return QAccessible::Button; // FIXME
//    case WebKit::ROLE_TOOLBAR:
//        return QAccessible::ToolBar;
//    case WebKit::ROLE_TOOLTIP:
//        return QAccessible::ToolTip;
//    case WebKit::ROLE_TREE:
//        return QAccessible::Tree;
//    case WebKit::ROLE_TREE_GRID:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_TREE_ITEM:
//        return QAccessible::TreeItem;
//    case WebKit::ROLE_VALUE_INDICATOR:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_WEBCORE_LINK:
//        return QAccessible::Link;
//    case WebKit::ROLE_WEB_AREA:
//        return QAccessible::NoRole; // FIXME
//    case WebKit::ROLE_WINDOW:
//        return QAccessible::Window;
    }

    qWarning() << "Unknown role: " << BrowserAccessibility::role();
    return QAccessible::NoRole;
}

QAccessible::State BrowserAccessibilityQt::state() const
{
    QAccessible::State state = QAccessible::State();
    int32 s = BrowserAccessibility::state();
    if (s & (1 << WebAXStateBusy))
        state.busy = true;
    if (s & (1 << WebAXStateChecked))
        state.checked = true;
    if (s & (1 << WebAXStateCollapsed))
        state.collapsed = true;
    if (!(s & (1 << WebAXStateEnabled)))
        state.disabled = true;
    if (s & (1 << WebAXStateExpanded))
        state.expanded = true;
    if (s & (1 << WebAXStateFocusable))
        state.focusable = true;
    if (s & (1 << WebAXStateFocused))
        state.focused = true;
    if (s & (1 << WebAXStateHaspopup))
        state.hasPopup = true;
    if (s & (1 << WebAXStateHovered))
        state.hotTracked = true;
    if (s & (1 << WebAXStateIndeterminate))
    {} // FIXME
    if (s & (1 << WebAXStateInvisible))
        state.invisible = true;
    if (s & (1 << WebAXStateLinked))
        state.linked = true;
    if (s & (1 << WebAXStateMultiselectable))
        state.multiSelectable = true;
    if (s & (1 << WebAXStateOffscreen))
        state.offscreen = true;
    if (s & (1 << WebAXStatePressed))
        state.pressed = true;
    if (s & (1 << WebAXStateProtected))
    {} // FIXME
    if (s & (1 << WebAXStateReadonly))
        state.readOnly = true;
    if (s & (1 << WebAXStateRequired))
    {} // FIXME
    if (s & (1 << WebAXStateSelectable))
        state.selectable = true;
    if (s & (1 << WebAXStateSelected))
        state.selected = true;
    if (s & (1 << WebAXStateVertical))
    {} // FIXME
    if (s & (1 << WebAXStateVisited))
    {} // FIXME
    return state;
}

void BrowserAccessibilityQt::NativeAddReference()
{
    QAccessible::registerAccessibleInterface(this);
}

void BrowserAccessibilityQt::NativeReleaseReference()
{
    QAccessible::Id interfaceId = QAccessible::uniqueId(this);
    QAccessible::deleteAccessibleInterface(interfaceId);
}

} // namespace content
