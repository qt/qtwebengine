/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "browser_accessibility_qt.h"

#include "third_party/WebKit/public/web/WebAXEnums.h"
#include "ui/accessibility/ax_node_data.h"

#include "browser_accessibility_manager_qt.h"
#include "qtwebenginecoreglobal.h"
#include "type_conversion.h"

using namespace blink;

namespace content {

BrowserAccessibilityQt::BrowserAccessibilityQt()
{
    QAccessible::registerAccessibleInterface(this);
}

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
    for (int i = 0; i < childCount(); ++i) {
        QAccessibleInterface *childIface = child(i);
        Q_ASSERT(childIface);
        if (childIface->rect().contains(x,y))
            return childIface;
    }
    return 0;
}

void *BrowserAccessibilityQt::interface_cast(QAccessible::InterfaceType type)
{
    switch (type) {
    case QAccessible::ActionInterface:
        if (!actionNames().isEmpty())
            return static_cast<QAccessibleActionInterface*>(this);
        break;
    case QAccessible::TextInterface:
        if (IsEditableText())
            return static_cast<QAccessibleTextInterface*>(this);
        break;
    case QAccessible::ValueInterface: {
        QAccessible::Role r = role();
        if (r == QAccessible::ProgressBar ||
                r == QAccessible::Slider ||
                r == QAccessible::ScrollBar ||
                r == QAccessible::SpinBox)
            return static_cast<QAccessibleValueInterface*>(this);
        break;
    }
    default:
        break;
    }
    return 0;
}

QAccessibleInterface *BrowserAccessibilityQt::parent() const
{
    BrowserAccessibility *p = GetParent();
    if (p)
        return static_cast<BrowserAccessibilityQt*>(p);
    return static_cast<BrowserAccessibilityManagerQt*>(manager())->rootParentAccessible();
}

QAccessibleInterface *BrowserAccessibilityQt::child(int index) const
{
    return static_cast<BrowserAccessibilityQt*>(BrowserAccessibility::PlatformGetChild(index));
}

int BrowserAccessibilityQt::childCount() const
{
    return PlatformChildCount();
}

int BrowserAccessibilityQt::indexOfChild(const QAccessibleInterface *iface) const
{

    const BrowserAccessibilityQt *child = static_cast<const BrowserAccessibilityQt*>(iface);
    return child->GetIndexInParent();
}

QString BrowserAccessibilityQt::text(QAccessible::Text t) const
{
    switch (t) {
    case QAccessible::Name:
        return toQt(GetStringAttribute(ui::AX_ATTR_NAME));
    case QAccessible::Description:
        return toQt(GetStringAttribute(ui::AX_ATTR_DESCRIPTION));
    case QAccessible::Help:
        return toQt(GetStringAttribute(ui::AX_ATTR_HELP));
    case QAccessible::Value:
        return toQt(GetStringAttribute(ui::AX_ATTR_VALUE));
    case QAccessible::Accelerator:
        return toQt(GetStringAttribute(ui::AX_ATTR_SHORTCUT));
    default:
        break;
    }
    return QString();
}

void BrowserAccessibilityQt::setText(QAccessible::Text t, const QString &text)
{
}

QRect BrowserAccessibilityQt::rect() const
{
    if (!manager()) // needed implicitly by GetGlobalBoundsRect()
        return QRect();
    gfx::Rect bounds = GetGlobalBoundsRect();
    return QRect(bounds.x(), bounds.y(), bounds.width(), bounds.height());
}

QAccessible::Role BrowserAccessibilityQt::role() const
{
    switch (GetRole()) {
    case ui::AX_ROLE_UNKNOWN:
        return QAccessible::NoRole;

    // Used by Chromium to distinguish between the root of the tree
    // for this page, and a web area for a frame within this page.
    case ui::AX_ROLE_WEB_AREA:
    case ui::AX_ROLE_ROOT_WEB_AREA: // not sure if we need to make a diff here, but this seems common
        return QAccessible::WebDocument;

    // These roles all directly correspond to blink accessibility roles,
    // keep these alphabetical.
    case ui::AX_ROLE_ALERT:
        return QAccessible::AlertMessage;
    case ui::AX_ROLE_ANNOTATION:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_APPLICATION:
        return QAccessible::Document; // returning Application here makes Qt return the top level app object
    case ui::AX_ROLE_ARTICLE:
        return QAccessible::Document; // FIXME
    case ui::AX_ROLE_BROWSER:
        return QAccessible::Document; // FIXME
    case ui::AX_ROLE_BUSY_INDICATOR:
        return QAccessible::Animation; // FIXME
    case ui::AX_ROLE_BUTTON:
        return QAccessible::Button;
    case ui::AX_ROLE_CANVAS:
        return QAccessible::Canvas;
    case ui::AX_ROLE_CELL:
        return QAccessible::Cell;
    case ui::AX_ROLE_CHECK_BOX:
        return QAccessible::CheckBox;
    case ui::AX_ROLE_COLOR_WELL:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_COLUMN:
        return QAccessible::Column;
    case ui::AX_ROLE_COLUMN_HEADER:
        return QAccessible::ColumnHeader;
    case ui::AX_ROLE_COMBO_BOX:
        return QAccessible::ComboBox;
    case ui::AX_ROLE_DEFINITION:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_DESCRIPTION_LIST_DETAIL:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_DESCRIPTION_LIST_TERM:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_DIALOG:
        return QAccessible::Dialog;
    case ui::AX_ROLE_DIRECTORY:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_DISCLOSURE_TRIANGLE:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_DIV:
        return QAccessible::Section;
    case ui::AX_ROLE_DOCUMENT:
        return QAccessible::Document;
    case ui::AX_ROLE_DRAWER:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_EDITABLE_TEXT:
        return QAccessible::EditableText;
    case ui::AX_ROLE_FOOTER:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_FORM:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_GRID:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_GROUP:
        return QAccessible::Grouping;
    case ui::AX_ROLE_GROW_AREA:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_HEADING:
        return QAccessible::StaticText; // FIXME
    case ui::AX_ROLE_HELP_TAG:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_HORIZONTAL_RULE:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_IGNORED:
        return QAccessible::NoRole;
    case ui::AX_ROLE_IMAGE:
        return QAccessible::Graphic;
    case ui::AX_ROLE_IMAGE_MAP:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_IMAGE_MAP_LINK:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_INCREMENTOR:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_INLINE_TEXT_BOX:
        return QAccessible::EditableText;
    case ui::AX_ROLE_LABEL_TEXT:
        return QAccessible::StaticText;
    case ui::AX_ROLE_LINK:
        return QAccessible::Link;
    case ui::AX_ROLE_LIST:
        return QAccessible::List;
    case ui::AX_ROLE_LIST_BOX:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_LIST_BOX_OPTION:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_LIST_ITEM:
        return QAccessible::ListItem;
    case ui::AX_ROLE_LIST_MARKER:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_LOG:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_MARQUEE:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_MATH:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_MATTE:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_MENU:
        return QAccessible::PopupMenu;
    case ui::AX_ROLE_MENU_BAR:
        return QAccessible::MenuBar;
    case ui::AX_ROLE_MENU_ITEM:
        return QAccessible::MenuItem;
    case ui::AX_ROLE_MENU_BUTTON:
        return QAccessible::MenuItem;
    case ui::AX_ROLE_MENU_LIST_OPTION:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_MENU_LIST_POPUP:
        return QAccessible::PopupMenu;
    case ui::AX_ROLE_NOTE:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_OUTLINE:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_PARAGRAPH:
        return QAccessible::Paragraph;
    case ui::AX_ROLE_PRESENTATIONAL:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_PROGRESS_INDICATOR:
        return QAccessible::ProgressBar;
    case ui::AX_ROLE_RADIO_BUTTON:
        return QAccessible::RadioButton;
    case ui::AX_ROLE_RADIO_GROUP:
        return QAccessible::RadioButton;
    case ui::AX_ROLE_REGION:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_ROW:
        return QAccessible::Row;
    case ui::AX_ROLE_ROW_HEADER:
        return QAccessible::RowHeader;
    case ui::AX_ROLE_RULER:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_RULER_MARKER:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_SCROLL_AREA:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_SCROLL_BAR:
        return QAccessible::ScrollBar;
    case ui::AX_ROLE_SHEET:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_SLIDER:
        return QAccessible::Slider;
    case ui::AX_ROLE_SLIDER_THUMB:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_SPIN_BUTTON:
        return QAccessible::SpinBox;
    case ui::AX_ROLE_SPIN_BUTTON_PART:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_SPLITTER:
        return QAccessible::Splitter;
    case ui::AX_ROLE_SPLIT_GROUP:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_STATIC_TEXT:
        return QAccessible::StaticText;
    case ui::AX_ROLE_STATUS:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_SVG_ROOT:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_SYSTEM_WIDE:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_TAB:
        return QAccessible::PageTab;
    case ui::AX_ROLE_TABLE:
        return QAccessible::Table;
    case ui::AX_ROLE_TABLE_HEADER_CONTAINER:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_TAB_GROUP:  // blink doesn't use (uses ROLE_TAB_LIST)
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_TAB_LIST:
        return QAccessible::PageTabList;
    case ui::AX_ROLE_TAB_PANEL:
        return QAccessible::PageTab;
    case ui::AX_ROLE_TEXT_AREA:
        return QAccessible::EditableText;
    case ui::AX_ROLE_TEXT_FIELD:
        return QAccessible::EditableText;
    case ui::AX_ROLE_TIMER:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_TOGGLE_BUTTON:
        return QAccessible::Button; // FIXME
    case ui::AX_ROLE_TOOLBAR:
        return QAccessible::ToolBar;
    case ui::AX_ROLE_TOOLTIP:
        return QAccessible::ToolTip;
    case ui::AX_ROLE_TREE:
        return QAccessible::Tree;
    case ui::AX_ROLE_TREE_GRID:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_TREE_ITEM:
        return QAccessible::TreeItem;
    case ui::AX_ROLE_VALUE_INDICATOR:
        return QAccessible::NoRole; // FIXME
    case ui::AX_ROLE_WINDOW:
        return QAccessible::Window;
    }
    return QAccessible::NoRole;
}

QAccessible::State BrowserAccessibilityQt::state() const
{
    QAccessible::State state = QAccessible::State();
    int32 s = GetState();
    if (s & (1 << ui::AX_STATE_BUSY))
        state.busy = true;
    if (s & (1 << ui::AX_STATE_CHECKED))
        state.checked = true;
    if (s & (1 << ui::AX_STATE_COLLAPSED))
        state.collapsed = true;
    if (s & (1 << ui::AX_STATE_DISABLED))
        state.disabled = true;
    if (s & (1 << ui::AX_STATE_EXPANDED))
        state.expanded = true;
    if (s & (1 << ui::AX_STATE_FOCUSABLE))
        state.focusable = true;
    if (s & (1 << ui::AX_STATE_FOCUSED))
        state.focused = true;
    if (s & (1 << ui::AX_STATE_HASPOPUP))
        state.hasPopup = true;
    if (s & (1 << ui::AX_STATE_HOVERED))
        state.hotTracked = true;
    if (s & (1 << ui::AX_STATE_INDETERMINATE))
    {} // FIXME
    if (s & (1 << ui::AX_STATE_INVISIBLE))
        state.invisible = true;
    if (s & (1 << ui::AX_STATE_LINKED))
        state.linked = true;
    if (s & (1 << ui::AX_STATE_MULTISELECTABLE))
        state.multiSelectable = true;
    if (s & (1 << ui::AX_STATE_OFFSCREEN))
        state.offscreen = true;
    if (s & (1 << ui::AX_STATE_PRESSED))
        state.pressed = true;
    if (s & (1 << ui::AX_STATE_PROTECTED))
    {} // FIXME
    if (s & (1 << ui::AX_STATE_READ_ONLY))
        state.readOnly = true;
    if (s & (1 << ui::AX_STATE_REQUIRED))
    {} // FIXME
    if (s & (1 << ui::AX_STATE_SELECTABLE))
        state.selectable = true;
    if (s & (1 << ui::AX_STATE_SELECTED))
        state.selected = true;
    if (s & (1 << ui::AX_STATE_VERTICAL))
    {} // FIXME
    if (s & (1 << ui::AX_STATE_VISITED))
    {} // FIXME
    if (IsEditableText())
        state.editable = true;
    return state;
}

// Qt does not reference count accessibles
void BrowserAccessibilityQt::NativeAddReference()
{
}

// there is no reference counting, but BrowserAccessibility::Destroy
// calls this (and that is the only place in the chromium sources,
// so we can safely use it to dispose of ourselves here
// (the default implementation of this function just contains a "delete this")
void BrowserAccessibilityQt::NativeReleaseReference()
{
    // delete this
    QAccessible::Id interfaceId = QAccessible::uniqueId(this);
    QAccessible::deleteAccessibleInterface(interfaceId);
}

QStringList BrowserAccessibilityQt::actionNames() const
{
    QStringList actions;
    if (HasState(ui::AX_STATE_FOCUSABLE))
        actions << QAccessibleActionInterface::setFocusAction();
    return actions;
}

void BrowserAccessibilityQt::doAction(const QString &actionName)
{
    if (actionName == QAccessibleActionInterface::setFocusAction())
        manager()->SetFocus(this, true);
}

QStringList BrowserAccessibilityQt::keyBindingsForAction(const QString &actionName) const
{
    QT_NOT_YET_IMPLEMENTED
    return QStringList();
}

void BrowserAccessibilityQt::addSelection(int startOffset, int endOffset)
{
    manager()->SetTextSelection(*this, startOffset, endOffset);
}

QString BrowserAccessibilityQt::attributes(int offset, int *startOffset, int *endOffset) const
{
    *startOffset = offset;
    *endOffset = offset;
    return QString();
}

int BrowserAccessibilityQt::cursorPosition() const
{
    int pos = 0;
    GetIntAttribute(ui::AX_ATTR_TEXT_SEL_START, &pos);
    return pos;
}

QRect BrowserAccessibilityQt::characterRect(int /*offset*/) const
{
    QT_NOT_YET_IMPLEMENTED
    return QRect();
}

int BrowserAccessibilityQt::selectionCount() const
{
    int start = 0;
    int end = 0;
    GetIntAttribute(ui::AX_ATTR_TEXT_SEL_START, &start);
    GetIntAttribute(ui::AX_ATTR_TEXT_SEL_END, &end);
    if (start != end)
        return 1;
    return 0;
}

int BrowserAccessibilityQt::offsetAtPoint(const QPoint &/*point*/) const
{
    QT_NOT_YET_IMPLEMENTED
    return 0;
}

void BrowserAccessibilityQt::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
    Q_ASSERT(startOffset && endOffset);
    *startOffset = 0;
    *endOffset = 0;
    if (selectionIndex != 0)
        return;
    GetIntAttribute(ui::AX_ATTR_TEXT_SEL_START, startOffset);
    GetIntAttribute(ui::AX_ATTR_TEXT_SEL_END, endOffset);
}

QString BrowserAccessibilityQt::text(int startOffset, int endOffset) const
{
    return text(QAccessible::Value).mid(startOffset, endOffset - startOffset);
}

void BrowserAccessibilityQt::removeSelection(int selectionIndex)
{
    manager()->SetTextSelection(*this, 0, 0);
}

void BrowserAccessibilityQt::setCursorPosition(int position)
{
    manager()->SetTextSelection(*this, position, position);
}

void BrowserAccessibilityQt::setSelection(int selectionIndex, int startOffset, int endOffset)
{
    if (selectionIndex != 0)
        return;
    manager()->SetTextSelection(*this, startOffset, endOffset);
}

int BrowserAccessibilityQt::characterCount() const
{
    return text(QAccessible::Value).length();
}

void BrowserAccessibilityQt::scrollToSubstring(int startIndex, int endIndex)
{
    int count = characterCount();
    if (startIndex < endIndex && endIndex < count)
        manager()->ScrollToMakeVisible(*this, GetLocalBoundsForRange(startIndex, endIndex - startIndex));
}

QVariant BrowserAccessibilityQt::currentValue() const
{
    QVariant result;
    float value;
    if (GetFloatAttribute(ui::AX_ATTR_VALUE_FOR_RANGE, &value)) {
        result = (double) value;
    }
    return result;
}

void BrowserAccessibilityQt::setCurrentValue(const QVariant &value)
{
    // not yet implemented anywhere in blink
    QT_NOT_YET_IMPLEMENTED
}

QVariant BrowserAccessibilityQt::maximumValue() const
{
    QVariant result;
    float value;
    if (GetFloatAttribute(ui::AX_ATTR_MAX_VALUE_FOR_RANGE, &value)) {
        result = (double) value;
    }
    return result;
}

QVariant BrowserAccessibilityQt::minimumValue() const
{
    QVariant result;
    float value;
    if (GetFloatAttribute(ui::AX_ATTR_MIN_VALUE_FOR_RANGE, &value)) {
        result = (double) value;
    }
    return result;
}

QVariant BrowserAccessibilityQt::minimumStepSize() const
{
    return QVariant();
}

} // namespace content
