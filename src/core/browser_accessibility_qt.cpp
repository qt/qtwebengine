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

#include "qtwebenginecoreglobal.h"
#include "content/common/accessibility_node_data.h"
#include "third_party/WebKit/public/web/WebAXEnums.h"
#include "type_conversion.h"
#include "browser_accessibility_manager_qt.h"

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
    return 0;
}

void *BrowserAccessibilityQt::interface_cast(QAccessible::InterfaceType type)
{
    switch (type) {
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
    BrowserAccessibility *p = BrowserAccessibility::parent();
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
    return child_count();
}

int BrowserAccessibilityQt::indexOfChild(const QAccessibleInterface *iface) const
{

    const BrowserAccessibilityQt *child = static_cast<const BrowserAccessibilityQt*>(iface);
    return child->index_in_parent();
}

QString BrowserAccessibilityQt::text(QAccessible::Text t) const
{
    switch (t) {
    case QAccessible::Name:
        return toQt(GetStringAttribute(AccessibilityNodeData::ATTR_NAME));
    case QAccessible::Description:
        return toQt(GetStringAttribute(AccessibilityNodeData::ATTR_DESCRIPTION));
    case QAccessible::Help:
        return toQt(GetStringAttribute(AccessibilityNodeData::ATTR_HELP));
    case QAccessible::Value:
        return toQt(GetStringAttribute(AccessibilityNodeData::ATTR_VALUE));
    case QAccessible::Accelerator:
        return toQt(GetStringAttribute(AccessibilityNodeData::ATTR_SHORTCUT));
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
    switch (BrowserAccessibility::role()) {
    case WebAXRoleUnknown:
        return QAccessible::NoRole;

    // Used by Chromium to distinguish between the root of the tree
    // for this page, and a web area for a frame within this page.
    case WebAXRoleWebArea:
    case WebAXRoleRootWebArea: // not sure if we need to make a diff here, but this seems common
        return QAccessible::Document;

    // These roles all directly correspond to blink accessibility roles,
    // keep these alphabetical.
    case WebAXRoleAlert:
        return QAccessible::AlertMessage;
    case WebAXRoleAnnotation:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleApplication:
        return QAccessible::Document; // returning Application here makes Qt return the top level app object
    case WebAXRoleArticle:
        return QAccessible::Document; // FIXME
    case WebAXRoleBrowser:
        return QAccessible::Document; // FIXME
    case WebAXRoleBusyIndicator:
        return QAccessible::Animation; // FIXME
    case WebAXRoleButton:
        return QAccessible::Button;
    case WebAXRoleCanvas:
        return QAccessible::Canvas;
    case WebAXRoleCell:
        return QAccessible::Cell;
    case WebAXRoleCheckBox:
        return QAccessible::CheckBox;
    case WebAXRoleColorWell:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleColumn:
        return QAccessible::Column;
    case WebAXRoleColumnHeader:
        return QAccessible::ColumnHeader;
    case WebAXRoleComboBox:
        return QAccessible::ComboBox;
    case WebAXRoleDefinition:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleDescriptionListDetail:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleDescriptionListTerm:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleDialog:
        return QAccessible::Dialog;
    case WebAXRoleDirectory:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleDisclosureTriangle:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleDiv:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleDocument:
        return QAccessible::Document;
    case WebAXRoleDrawer:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleEditableText:
        return QAccessible::EditableText;
    case WebAXRoleFooter:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleForm:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleGrid:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleGroup:
        return QAccessible::Grouping;
    case WebAXRoleGrowArea:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleHeading:
        return QAccessible::StaticText; // FIXME
    case WebAXRoleHelpTag:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleHorizontalRule:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleIgnored:
        return QAccessible::NoRole;
    case WebAXRoleImage:
        return QAccessible::Graphic;
    case WebAXRoleImageMap:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleImageMapLink:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleIncrementor:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleLabel:
        return QAccessible::StaticText;
    case WebAXRoleLink:
        return QAccessible::Link;
    case WebAXRoleList:
        return QAccessible::List;
    case WebAXRoleListBox:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleListBoxOption:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleListItem:
        return QAccessible::ListItem;
    case WebAXRoleListMarker:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleLog:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleMarquee:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleMath:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleMatte:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleMenu:
        return QAccessible::PopupMenu;
    case WebAXRoleMenuBar:
        return QAccessible::MenuBar;
    case WebAXRoleMenuItem:
        return QAccessible::MenuItem;
    case WebAXRoleMenuButton:
        return QAccessible::MenuItem;
    case WebAXRoleMenuListOption:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleMenuListPopup:
        return QAccessible::PopupMenu;
    case WebAXRoleNote:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleOutline:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleParagraph:
        return QAccessible::NoRole; // FIXME
    case WebAXRolePopUpButton:
        return QAccessible::NoRole; // FIXME
    case WebAXRolePresentational:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleProgressIndicator:
        return QAccessible::ProgressBar;
    case WebAXRoleRadioButton:
        return QAccessible::RadioButton;
    case WebAXRoleRadioGroup:
        return QAccessible::RadioButton;
    case WebAXRoleRegion:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleRow:
        return QAccessible::Row;
    case WebAXRoleRowHeader:
        return QAccessible::RowHeader;
    case WebAXRoleRuler:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleRulerMarker:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleScrollArea:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleScrollBar:
        return QAccessible::ScrollBar;
    case WebAXRoleSheet:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleSlider:
        return QAccessible::Slider;
    case WebAXRoleSliderThumb:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleSpinButton:
        return QAccessible::SpinBox;
    case WebAXRoleSpinButtonPart:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleSplitter:
        return QAccessible::Splitter;
    case WebAXRoleSplitGroup:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleStaticText:
        return QAccessible::StaticText;
    case WebAXRoleStatus:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleSVGRoot:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleSystemWide:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleTab:
        return QAccessible::PageTab;
    case WebAXRoleTable:
        return QAccessible::Table;
    case WebAXRoleTableHeaderContainer:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleTabGroup:  // blink doesn't use (uses ROLE_TAB_LIST)
        return QAccessible::NoRole; // FIXME
    case WebAXRoleTabList:
        return QAccessible::PageTabList;
    case WebAXRoleTabPanel:
        return QAccessible::PageTab;
    case WebAXRoleTextArea:
        return QAccessible::EditableText;
    case WebAXRoleTextField:
        return QAccessible::EditableText;
    case WebAXRoleTimer:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleToggleButton:
        return QAccessible::Button; // FIXME
    case WebAXRoleToolbar:
        return QAccessible::ToolBar;
    case WebAXRoleUserInterfaceTooltip:
        return QAccessible::ToolTip;
    case WebAXRoleTree:
        return QAccessible::Tree;
    case WebAXRoleTreeGrid:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleTreeItem:
        return QAccessible::TreeItem;
    case WebAXRoleValueIndicator:
        return QAccessible::NoRole; // FIXME
    case WebAXRoleWindow:
        return QAccessible::Window;
    }
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
    GetIntAttribute(AccessibilityNodeData::ATTR_TEXT_SEL_START, &pos);
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
    GetIntAttribute(AccessibilityNodeData::ATTR_TEXT_SEL_START, &start);
    GetIntAttribute(AccessibilityNodeData::ATTR_TEXT_SEL_END, &end);
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
    GetIntAttribute(AccessibilityNodeData::ATTR_TEXT_SEL_START, startOffset);
    GetIntAttribute(AccessibilityNodeData::ATTR_TEXT_SEL_END, endOffset);
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
    if (GetFloatAttribute(AccessibilityNodeData::ATTR_VALUE_FOR_RANGE, &value)) {
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
    if (GetFloatAttribute(AccessibilityNodeData::ATTR_MAX_VALUE_FOR_RANGE, &value)) {
        result = (double) value;
    }
    return result;
}

QVariant BrowserAccessibilityQt::minimumValue() const
{
    QVariant result;
    float value;
    if (GetFloatAttribute(AccessibilityNodeData::ATTR_MIN_VALUE_FOR_RANGE, &value)) {
        result = (double) value;
    }
    return result;
}

QVariant BrowserAccessibilityQt::minimumStepSize() const
{
    return QVariant();
}

} // namespace content
