
#include "browser_accessibility_qt.h"

#include "content/common/accessibility_node_data.h"

#include "type_conversion.h"
#include "browser_accessibility_manager_qt.h"

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
    string16 name_str = name_;
    if (name_str.empty()) {
        int title_elem_id;
        if (GetIntAttribute(AccessibilityNodeData::ATTR_TITLE_UI_ELEMENT,
                                    &title_elem_id)) {
            BrowserAccessibility* title_elem =
                    manager_->GetFromRendererID(title_elem_id);
            if (title_elem)
                name_str = title_elem->GetTextRecursive();
        }
    }
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
    case content::AccessibilityNodeData::ROLE_UNKNOWN:
        return QAccessible::NoRole;

    // Used by Chromium to distinguish between the root of the tree
    // for this page, and a web area for a frame within this page.
    case content::AccessibilityNodeData::ROLE_ROOT_WEB_AREA:
        return QAccessible::Document;

    // These roles all directly correspond to WebKit accessibility roles,
    // keep these alphabetical.
    case content::AccessibilityNodeData::ROLE_ALERT:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_ALERT_DIALOG:
        return QAccessible::AlertMessage;
    case content::AccessibilityNodeData::ROLE_ANNOTATION:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_APPLICATION:
        return QAccessible::Application;
    case content::AccessibilityNodeData::ROLE_ARTICLE:
        return QAccessible::Document; // FIXME
    case content::AccessibilityNodeData::ROLE_BROWSER:
        return QAccessible::Document; // FIXME
    case content::AccessibilityNodeData::ROLE_BUSY_INDICATOR:
        return QAccessible::Animation; // FIXME
    case content::AccessibilityNodeData::ROLE_BUTTON:
        return QAccessible::Button;
    case content::AccessibilityNodeData::ROLE_CANVAS:
        return QAccessible::Canvas;
    case content::AccessibilityNodeData::ROLE_CANVAS_WITH_FALLBACK_CONTENT:
        return QAccessible::Canvas;
    case content::AccessibilityNodeData::ROLE_CELL:
        return QAccessible::Cell;
    case content::AccessibilityNodeData::ROLE_CHECKBOX:
        return QAccessible::CheckBox;
    case content::AccessibilityNodeData::ROLE_COLOR_WELL:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_COLUMN:
        return QAccessible::Column;
    case content::AccessibilityNodeData::ROLE_COLUMN_HEADER:
        return QAccessible::ColumnHeader;
    case content::AccessibilityNodeData::ROLE_COMBO_BOX:
        return QAccessible::ComboBox;
    case content::AccessibilityNodeData::ROLE_DEFINITION:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_DESCRIPTION_LIST_DETAIL:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_DESCRIPTION_LIST_TERM:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_DIALOG:
        return QAccessible::Dialog;
    case content::AccessibilityNodeData::ROLE_DIRECTORY:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_DISCLOSURE_TRIANGLE:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_DIV:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_DOCUMENT:
        return QAccessible::Document;
    case content::AccessibilityNodeData::ROLE_DRAWER:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_EDITABLE_TEXT:
        return QAccessible::EditableText;
    case content::AccessibilityNodeData::ROLE_FOOTER:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_FORM:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_GRID:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_GROUP:
        return QAccessible::Grouping;
    case content::AccessibilityNodeData::ROLE_GROW_AREA:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_HEADING:
        return QAccessible::StaticText; // FIXME
    case content::AccessibilityNodeData::ROLE_HELP_TAG:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_HORIZONTAL_RULE:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_IGNORED:
        return QAccessible::NoRole;
    case content::AccessibilityNodeData::ROLE_IMAGE:
        return QAccessible::Graphic;
    case content::AccessibilityNodeData::ROLE_IMAGE_MAP:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_IMAGE_MAP_LINK:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_INCREMENTOR:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LABEL:
        return QAccessible::StaticText;
    case content::AccessibilityNodeData::ROLE_LANDMARK_APPLICATION:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LANDMARK_BANNER:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LANDMARK_COMPLEMENTARY:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LANDMARK_CONTENTINFO:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LANDMARK_MAIN:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LANDMARK_NAVIGATION:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LANDMARK_SEARCH:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LINK:
        return QAccessible::Link;
    case content::AccessibilityNodeData::ROLE_LIST:
        return QAccessible::List;
    case content::AccessibilityNodeData::ROLE_LISTBOX:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LISTBOX_OPTION:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LIST_ITEM:
        return QAccessible::ListItem;
    case content::AccessibilityNodeData::ROLE_LIST_MARKER:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_LOG:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_MARQUEE:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_MATH:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_MATTE:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_MENU:
        return QAccessible::PopupMenu;
    case content::AccessibilityNodeData::ROLE_MENU_BAR:
        return QAccessible::MenuBar;
    case content::AccessibilityNodeData::ROLE_MENU_ITEM:
        return QAccessible::MenuItem;
    case content::AccessibilityNodeData::ROLE_MENU_BUTTON:
        return QAccessible::MenuItem;
    case content::AccessibilityNodeData::ROLE_MENU_LIST_OPTION:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_MENU_LIST_POPUP:
        return QAccessible::PopupMenu;
    case content::AccessibilityNodeData::ROLE_NOTE:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_OUTLINE:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_PARAGRAPH:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_POPUP_BUTTON:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_PRESENTATIONAL:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_PROGRESS_INDICATOR:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_RADIO_BUTTON:
        return QAccessible::RadioButton;
    case content::AccessibilityNodeData::ROLE_RADIO_GROUP:
        return QAccessible::RadioButton;
    case content::AccessibilityNodeData::ROLE_REGION:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_ROW:
        return QAccessible::Row;
    case content::AccessibilityNodeData::ROLE_ROW_HEADER:
        return QAccessible::RowHeader;
    case content::AccessibilityNodeData::ROLE_RULER:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_RULER_MARKER:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_SCROLLAREA:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_SCROLLBAR:
        return QAccessible::ScrollBar;
    case content::AccessibilityNodeData::ROLE_SHEET:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_SLIDER:
        return QAccessible::Slider;
    case content::AccessibilityNodeData::ROLE_SLIDER_THUMB:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_SPIN_BUTTON:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_SPIN_BUTTON_PART:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_SPLITTER:
        return QAccessible::Splitter;
    case content::AccessibilityNodeData::ROLE_SPLIT_GROUP:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_STATIC_TEXT:
        return QAccessible::StaticText;
    case content::AccessibilityNodeData::ROLE_STATUS:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_SVG_ROOT:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_SYSTEM_WIDE:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_TAB:
        return QAccessible::PageTab;
    case content::AccessibilityNodeData::ROLE_TABLE:
        return QAccessible::Table;
    case content::AccessibilityNodeData::ROLE_TABLE_HEADER_CONTAINER:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_TAB_GROUP_UNUSED:  // WebKit doesn't use (uses ROLE_TAB_LIST)
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_TAB_LIST:
        return QAccessible::PageTabList;
    case content::AccessibilityNodeData::ROLE_TAB_PANEL:
        return QAccessible::PageTab;
    case content::AccessibilityNodeData::ROLE_TEXTAREA:
        return QAccessible::EditableText;
    case content::AccessibilityNodeData::ROLE_TEXT_FIELD:
        return QAccessible::EditableText;
    case content::AccessibilityNodeData::ROLE_TIMER:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_TOGGLE_BUTTON:
        return QAccessible::Button; // FIXME
    case content::AccessibilityNodeData::ROLE_TOOLBAR:
        return QAccessible::ToolBar;
    case content::AccessibilityNodeData::ROLE_TOOLTIP:
        return QAccessible::ToolTip;
    case content::AccessibilityNodeData::ROLE_TREE:
        return QAccessible::Tree;
    case content::AccessibilityNodeData::ROLE_TREE_GRID:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_TREE_ITEM:
        return QAccessible::TreeItem;
    case content::AccessibilityNodeData::ROLE_VALUE_INDICATOR:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_WEBCORE_LINK:
        return QAccessible::Link;
    case content::AccessibilityNodeData::ROLE_WEB_AREA:
        return QAccessible::NoRole; // FIXME
    case content::AccessibilityNodeData::ROLE_WINDOW:
        return QAccessible::Window;
        break;
    }

    return QAccessible::Button;
}

QAccessible::State BrowserAccessibilityQt::state() const
{
    QAccessible::State state = QAccessible::State();
    int32 s = BrowserAccessibility::state();
    if (s & (1 << AccessibilityNodeData::STATE_BUSY))
        state.busy = true;
    if (s & (1 << AccessibilityNodeData::STATE_CHECKED))
        state.checked = true;
    if (s & (1 << AccessibilityNodeData::STATE_COLLAPSED))
        state.collapsed = true;
    if (s & (1 << AccessibilityNodeData::STATE_EXPANDED))
        state.expanded = true;
    if (s & (1 << AccessibilityNodeData::STATE_FOCUSABLE))
        state.focusable = true;
    if (s & (1 << AccessibilityNodeData::STATE_FOCUSED))
        state.focused = true;
    if (s & (1 << AccessibilityNodeData::STATE_HASPOPUP))
        state.hasPopup = true;
    if (s & (1 << AccessibilityNodeData::STATE_HOTTRACKED))
        state.hotTracked = true;
    if (s & (1 << AccessibilityNodeData::STATE_INDETERMINATE))
    {} // FIXME
    if (s & (1 << AccessibilityNodeData::STATE_INVISIBLE))
        state.invisible = true;
    if (s & (1 << AccessibilityNodeData::STATE_LINKED))
        state.linked = true;
    if (s & (1 << AccessibilityNodeData::STATE_MULTISELECTABLE))
        state.multiSelectable = true;
    if (s & (1 << AccessibilityNodeData::STATE_OFFSCREEN))
        state.offscreen = true;
    if (s & (1 << AccessibilityNodeData::STATE_PRESSED))
        state.pressed = true;
    if (s & (1 << AccessibilityNodeData::STATE_PROTECTED))
    {} // FIXME
    if (s & (1 << AccessibilityNodeData::STATE_READONLY))
        state.readOnly = true;
    if (s & (1 << AccessibilityNodeData::STATE_REQUIRED))
    {} // FIXME
    if (s & (1 << AccessibilityNodeData::STATE_SELECTABLE))
        state.selectable = true;
    if (s & (1 << AccessibilityNodeData::STATE_SELECTED))
        state.selected = true;
    if (s & (1 << AccessibilityNodeData::STATE_TRAVERSED))
        state.traversed = true;
    if (s & (1 << AccessibilityNodeData::STATE_UNAVAILABLE))
        state.invalid = true; // FIXME does this make sense?
    if (s & (1 << AccessibilityNodeData::STATE_VERTICAL))
    {} // FIXME
    if (s & (1 << AccessibilityNodeData::STATE_VISITED))
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
