// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "browser_accessibility_qt.h"
#include "browser_accessibility_manager_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "type_conversion.h"

#include "content/browser/accessibility/browser_accessibility.h"
#include "ui/accessibility/ax_enums.mojom.h"

#include <QtGui/qaccessible.h>

namespace QtWebEngineCore {
class BrowserAccessibilityInterface;

class BrowserAccessibilityQt
    : public content::BrowserAccessibility
{
public:
    BrowserAccessibilityQt(content::BrowserAccessibilityManager *manager, ui::AXNode *node);
    ~BrowserAccessibilityQt();

    bool isReady() const;

    QtWebEngineCore::BrowserAccessibilityInterface *interface = nullptr;
};

class BrowserAccessibilityInterface
    : public QAccessibleInterface
    , public QAccessibleActionInterface
    , public QAccessibleTextInterface
    , public QAccessibleValueInterface
    , public QAccessibleTableInterface
    , public QAccessibleTableCellInterface
{
public:
    BrowserAccessibilityInterface(BrowserAccessibilityQt *chromiumInterface);
    ~BrowserAccessibilityInterface() override;

    void destroy();

    // QAccessibleInterface
    bool isValid() const override;
    QObject *object() const override;
    QAccessibleInterface *childAt(int x, int y) const override;
    void *interface_cast(QAccessible::InterfaceType type) override;

    // navigation, hierarchy
    QAccessibleInterface *parent() const override;
    QAccessibleInterface *child(int index) const override;
    QAccessibleInterface *focusChild() const override;
    int childCount() const override;
    int indexOfChild(const QAccessibleInterface *) const override;

    // properties and state
    QString text(QAccessible::Text t) const override;
    void setText(QAccessible::Text t, const QString &text) override;
    QRect rect() const override;
    QAccessible::Role role() const override;
    QAccessible::State state() const override;

    // QAccessibleActionInterface
    QStringList actionNames() const override;
    void doAction(const QString &actionName) override;
    QStringList keyBindingsForAction(const QString &actionName) const override;

    // QAccessibleTextInterface
    void addSelection(int startOffset, int endOffset) override;
    QString attributes(int offset, int *startOffset, int *endOffset) const override;
    int cursorPosition() const override;
    QRect characterRect(int offset) const override;
    int selectionCount() const override;
    int offsetAtPoint(const QPoint &point) const override;
    void selection(int selectionIndex, int *startOffset, int *endOffset) const override;
    QString text(int startOffset, int endOffset) const override;
    void removeSelection(int selectionIndex) override;
    void setCursorPosition(int position) override;
    void setSelection(int selectionIndex, int startOffset, int endOffset) override;
    int characterCount() const override;
    void scrollToSubstring(int startIndex, int endIndex) override;

    // QAccessibleValueInterface
    QVariant currentValue() const override;
    void setCurrentValue(const QVariant &value) override;
    QVariant maximumValue() const override;
    QVariant minimumValue() const override;
    QVariant minimumStepSize() const override;

    // QAccessibleTableInterface
    QAccessibleInterface *cellAt(int row, int column) const override;
    QAccessibleInterface *caption() const override;
    QAccessibleInterface *summary() const override;
    QString columnDescription(int column) const override;
    QString rowDescription(int row) const override;
    int columnCount() const override;
    int rowCount() const override;
    // selection
    int selectedCellCount() const override;
    int selectedColumnCount() const override;
    int selectedRowCount() const override;
    QList<QAccessibleInterface *> selectedCells() const override;
    QList<int> selectedColumns() const override;
    QList<int> selectedRows() const override;
    bool isColumnSelected(int column) const override;
    bool isRowSelected(int row) const override;
    bool selectRow(int row) override;
    bool selectColumn(int column) override;
    bool unselectRow(int row) override;
    bool unselectColumn(int column) override;

    // QAccessibleTableCellInterface
    int columnExtent() const override;
    QList<QAccessibleInterface *> columnHeaderCells() const override;
    int columnIndex() const override;
    int rowExtent() const override;
    QList<QAccessibleInterface *> rowHeaderCells() const override;
    int rowIndex() const override;
    bool isSelected() const override;
    QAccessibleInterface *table() const override;

    void modelChange(QAccessibleTableModelChangeEvent *event) override;

private:
    content::BrowserAccessibility *findTable() const;

    QObject *m_object = nullptr;
    QAccessible::Id m_id = 0;
    BrowserAccessibilityQt *q;
};

BrowserAccessibilityQt::BrowserAccessibilityQt(content::BrowserAccessibilityManager *manager,
                                               ui::AXNode *node)
    : content::BrowserAccessibility(manager, node)
    , interface(new BrowserAccessibilityInterface(this))
{
}

BrowserAccessibilityQt::~BrowserAccessibilityQt()
{
    if (interface)
        interface->destroy();
}

bool BrowserAccessibilityQt::isReady() const
{
    // FIXME: This is just a workaround, remove this when the commented out assert in
    //        BrowserAccessibilityManager::GetFromID(int32_t id) gets fixed.
    return manager()->GetFromID(node()->id()) != nullptr;
}

BrowserAccessibilityInterface::BrowserAccessibilityInterface(BrowserAccessibilityQt *chromiumInterface)
    : q(chromiumInterface)
{
    if (parent() && parent()->object()) {
        m_object = new QObject(parent()->object());
        QString name = toQt(q->GetAuthorUniqueId());
        if (!name.isEmpty())
            m_object->setObjectName(name);
    }

    m_id = QAccessible::registerAccessibleInterface(this);
}

BrowserAccessibilityInterface::~BrowserAccessibilityInterface()
{
    q->interface = nullptr;
}

void BrowserAccessibilityInterface::destroy()
{
    QAccessible::deleteAccessibleInterface(m_id);
}

bool BrowserAccessibilityInterface::isValid() const
{
    if (!q->isReady())
        return false;

    auto managerQt = static_cast<content::BrowserAccessibilityManagerQt *>(q->manager());
    return managerQt && managerQt->isValid();
}

QObject *BrowserAccessibilityInterface::object() const
{
    return m_object;
}

QAccessibleInterface *BrowserAccessibilityInterface::childAt(int x, int y) const
{
    for (int i = 0; i < childCount(); ++i) {
        QAccessibleInterface *childIface = child(i);
        Q_ASSERT(childIface);
        if (childIface->rect().contains(x,y))
            return childIface;
    }
    return nullptr;
}

void *BrowserAccessibilityInterface::interface_cast(QAccessible::InterfaceType type)
{
    switch (type) {
    case QAccessible::ActionInterface:
        if (!actionNames().isEmpty())
            return static_cast<QAccessibleActionInterface*>(this);
        break;
    case QAccessible::TextInterface:
        if (q->HasState(ax::mojom::State::kEditable))
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
    case QAccessible::TableInterface: {
        QAccessible::Role r = role();
        if (r == QAccessible::Table ||
            r == QAccessible::List ||
            r == QAccessible::Tree)
            return static_cast<QAccessibleTableInterface*>(this);
        break;
    }
    case QAccessible::TableCellInterface: {
        QAccessible::Role r = role();
        if (r == QAccessible::Cell || r == QAccessible::ListItem || r == QAccessible::TreeItem) {
            if (findTable())
                return static_cast<QAccessibleTableCellInterface *>(this);
        }
        break;
    }
    default:
        break;
    }
    return nullptr;
}

QAccessibleInterface *BrowserAccessibilityInterface::parent() const
{
    content::BrowserAccessibility *chromiumParent = q->PlatformGetParent();
    if (chromiumParent)
        return toQAccessibleInterface(chromiumParent);
    return static_cast<content::BrowserAccessibilityManagerQt*>(q->manager())->rootParentAccessible();
}

QAccessibleInterface *BrowserAccessibilityInterface::child(int index) const
{
    content::BrowserAccessibility *chromiumChild = q->PlatformGetChild(index);
    return chromiumChild ? toQAccessibleInterface(chromiumChild) : nullptr;
}

QAccessibleInterface *BrowserAccessibilityInterface::focusChild() const
{
    if (state().focused)
        return const_cast<BrowserAccessibilityInterface *>(this);

    for (int i = 0; i < childCount(); ++i) {
        if (QAccessibleInterface *iface = child(i)->focusChild())
            return iface;
    }

    return nullptr;
}

int BrowserAccessibilityInterface::childCount() const
{
    return q->PlatformChildCount();
}

int BrowserAccessibilityInterface::indexOfChild(const QAccessibleInterface *iface) const
{

    const BrowserAccessibilityInterface *child = static_cast<const BrowserAccessibilityInterface *>(iface);
    return const_cast<BrowserAccessibilityInterface *>(child)->q->GetIndexInParent().value();
}

QString BrowserAccessibilityInterface::text(QAccessible::Text t) const
{
    if (!q->isReady())
        return QString();

    switch (t) {
    case QAccessible::Name:
        return toQt(q->GetStringAttribute(ax::mojom::StringAttribute::kName));
    case QAccessible::Description:
        return toQt(q->GetStringAttribute(ax::mojom::StringAttribute::kDescription));
    case QAccessible::Value:
        return toQt(q->GetStringAttribute(ax::mojom::StringAttribute::kValue));
    case QAccessible::Accelerator:
        return toQt(q->GetStringAttribute(ax::mojom::StringAttribute::kKeyShortcuts));
    default:
        break;
    }
    return QString();
}

void BrowserAccessibilityInterface::setText(QAccessible::Text t, const QString &text)
{
}

QRect BrowserAccessibilityInterface::rect() const
{
    if (!q->manager() || !q->isReady()) // needed implicitly by GetScreenBoundsRect()
        return QRect();
    gfx::Rect bounds = q->GetUnclippedScreenBoundsRect();
    bounds = gfx::ScaleToRoundedRect(bounds, 1.f / q->manager()->device_scale_factor()); // FIXME: check
    return QRect(bounds.x(), bounds.y(), bounds.width(), bounds.height());
}

QAccessible::Role BrowserAccessibilityInterface::role() const
{
    switch (q->GetRole()) {
    case ax::mojom::Role::kNone:
    case ax::mojom::Role::kUnknown:
        return QAccessible::NoRole;

    // Internal roles (matching auralinux and win)
    case ax::mojom::Role::kKeyboard:
    case ax::mojom::Role::kImeCandidate:
        return QAccessible::NoRole;

    // Used by Chromium to distinguish between the root of the tree
    // for this page, and a web area for a frame within this page.
    case ax::mojom::Role::kWebView:
    case ax::mojom::Role::kRootWebArea: // not sure if we need to make a diff here, but this seems common
        return QAccessible::WebDocument;

    // These roles all directly correspond to blink accessibility roles,
    // keep these alphabetical.
    case ax::mojom::Role::kAbbr:
        return QAccessible::StaticText;
    case ax::mojom::Role::kAlert:
    case ax::mojom::Role::kAlertDialog:
        return QAccessible::AlertMessage;
    case ax::mojom::Role::kApplication:
        return QAccessible::Document; // returning Application here makes Qt return the top level app object
    case ax::mojom::Role::kArticle:
        return QAccessible::Section;
    case ax::mojom::Role::kAudio:
        return QAccessible::Sound;
    case ax::mojom::Role::kBanner:
        return QAccessible::Section;
    case ax::mojom::Role::kBlockquote:
        return QAccessible::Section;
    case ax::mojom::Role::kButton:
        return QAccessible::Button;
    case ax::mojom::Role::kCanvas:
        return QAccessible::Canvas;
    case ax::mojom::Role::kCaption:
        return QAccessible::Heading;
    case ax::mojom::Role::kCaret:
        return QAccessible::NoRole; // FIXME: https://codereview.chromium.org/2781613003
    case ax::mojom::Role::kCell:
        return QAccessible::Cell;
    case ax::mojom::Role::kCheckBox:
        return QAccessible::CheckBox;
    case ax::mojom::Role::kClient:
        return QAccessible::Client;
    case ax::mojom::Role::kCode:
        return QAccessible::StaticText;
    case ax::mojom::Role::kColorWell:
        return QAccessible::ColorChooser;
    case ax::mojom::Role::kColumn:
        return QAccessible::Column;
    case ax::mojom::Role::kColumnHeader:
        return QAccessible::ColumnHeader;
    case ax::mojom::Role::kComboBoxGrouping:
    case ax::mojom::Role::kComboBoxMenuButton:
    case ax::mojom::Role::kTextFieldWithComboBox:
        return QAccessible::ComboBox;
    case ax::mojom::Role::kComboBoxSelect:
        return QAccessible::PopupMenu;
    case ax::mojom::Role::kComplementary:
        return QAccessible::ComplementaryContent;
    case ax::mojom::Role::kComment:
        return QAccessible::Section;
    case ax::mojom::Role::kContentDeletion:
    case ax::mojom::Role::kContentInsertion:
        return QAccessible::Grouping;
    case ax::mojom::Role::kContentInfo:
        return QAccessible::Section;
    case ax::mojom::Role::kDate:
    case ax::mojom::Role::kDateTime:
        return QAccessible::Clock;
    case ax::mojom::Role::kDefinition:
        return QAccessible::Paragraph;
    case ax::mojom::Role::kDescriptionList:
        return QAccessible::List;
    case ax::mojom::Role::kDescriptionListDetail:
        return QAccessible::Paragraph;
    case ax::mojom::Role::kDescriptionListTerm:
        return QAccessible::ListItem;
    case ax::mojom::Role::kDetails:
        return QAccessible::Grouping;
    case ax::mojom::Role::kDesktop:
        return QAccessible::Pane;
    case ax::mojom::Role::kDialog:
        return QAccessible::Dialog;
    case ax::mojom::Role::kDirectory:
        return QAccessible::List;
    case ax::mojom::Role::kDisclosureTriangle:
        return QAccessible::Button;
    case ax::mojom::Role::kGenericContainer:
        return QAccessible::Section;
    case ax::mojom::Role::kDocCover:
        return QAccessible::Graphic;
    case ax::mojom::Role::kDocBackLink:
    case ax::mojom::Role::kDocBiblioRef:
    case ax::mojom::Role::kDocGlossRef:
    case ax::mojom::Role::kDocNoteRef:
        return QAccessible::Link;
    case ax::mojom::Role::kDocBiblioEntry:
    case ax::mojom::Role::kDocEndnote:
    case ax::mojom::Role::kDocFootnote:
        return QAccessible::ListItem;
    case ax::mojom::Role::kDocPageBreak:
        return QAccessible::Separator;
    case ax::mojom::Role::kDocAbstract:
    case ax::mojom::Role::kDocAcknowledgments:
    case ax::mojom::Role::kDocAfterword:
    case ax::mojom::Role::kDocAppendix:
    case ax::mojom::Role::kDocBibliography:
    case ax::mojom::Role::kDocChapter:
    case ax::mojom::Role::kDocColophon:
    case ax::mojom::Role::kDocConclusion:
    case ax::mojom::Role::kDocCredit:
    case ax::mojom::Role::kDocCredits:
    case ax::mojom::Role::kDocDedication:
    case ax::mojom::Role::kDocEndnotes:
    case ax::mojom::Role::kDocEpigraph:
    case ax::mojom::Role::kDocEpilogue:
    case ax::mojom::Role::kDocErrata:
    case ax::mojom::Role::kDocExample:
    case ax::mojom::Role::kDocForeword:
    case ax::mojom::Role::kDocGlossary:
    case ax::mojom::Role::kDocIndex:
    case ax::mojom::Role::kDocIntroduction:
    case ax::mojom::Role::kDocNotice:
        return QAccessible::Section;
    case ax::mojom::Role::kDocPageFooter:
        return QAccessible::Footer;
    case ax::mojom::Role::kDocPageHeader:
        return QAccessible::Heading;
    case ax::mojom::Role::kDocPageList:
    case ax::mojom::Role::kDocPart:
    case ax::mojom::Role::kDocPreface:
    case ax::mojom::Role::kDocPrologue:
    case ax::mojom::Role::kDocPullquote:
    case ax::mojom::Role::kDocQna:
        return QAccessible::Section;
    case ax::mojom::Role::kDocSubtitle:
        return QAccessible::Heading;
    case ax::mojom::Role::kDocTip:
    case ax::mojom::Role::kDocToc:
        return QAccessible::Section;
    case ax::mojom::Role::kDocument:
        return QAccessible::Document;
    case ax::mojom::Role::kEmbeddedObject:
        return QAccessible::Grouping;
    case ax::mojom::Role::kEmphasis:
        return QAccessible::StaticText;
    case ax::mojom::Role::kFeed:
        return QAccessible::Section;
    case ax::mojom::Role::kFigcaption:
        return QAccessible::Heading;
    case ax::mojom::Role::kFigure:
        return QAccessible::Section;
    case ax::mojom::Role::kFooter:
        // CORE-AAM recommends LANDMARK instead of FOOTER.
        return QAccessible::Section;
    case ax::mojom::Role::kFooterAsNonLandmark:
        return QAccessible::Section;
    case ax::mojom::Role::kForm:
        return QAccessible::Form;
    case ax::mojom::Role::kGraphicsDocument:
        return QAccessible::Document;
    case ax::mojom::Role::kGraphicsObject:
        return QAccessible::Pane;
    case ax::mojom::Role::kGraphicsSymbol:
        return QAccessible::Graphic;
    case ax::mojom::Role::kGrid:
        return QAccessible::Table;
    case ax::mojom::Role::kGroup:
        return QAccessible::Grouping;
    case ax::mojom::Role::kHeader:
    case ax::mojom::Role::kHeaderAsNonLandmark:
        return QAccessible::Section;
    case ax::mojom::Role::kHeading:
        return QAccessible::Heading;
    case ax::mojom::Role::kIframe:
        return QAccessible::WebDocument;
    case ax::mojom::Role::kIframePresentational:
        return QAccessible::Grouping;
    case ax::mojom::Role::kImage:
        return QAccessible::Graphic;
    case ax::mojom::Role::kInlineTextBox:
        return QAccessible::StaticText;
    case ax::mojom::Role::kInputTime:
        return QAccessible::SpinBox;
    case ax::mojom::Role::kLabelText:
        return QAccessible::StaticText;
    case ax::mojom::Role::kLayoutTable:
    case ax::mojom::Role::kLayoutTableCell:
    case ax::mojom::Role::kLayoutTableRow:
        return QAccessible::Section;
    case ax::mojom::Role::kLegend:
        return QAccessible::StaticText;
    case ax::mojom::Role::kLineBreak:
        return QAccessible::Separator;
    case ax::mojom::Role::kLink:
        return QAccessible::Link;
    case ax::mojom::Role::kList:
        return QAccessible::List;
    case ax::mojom::Role::kListBox:
        return QAccessible::ComboBox;
    case ax::mojom::Role::kListBoxOption:
        return QAccessible::ListItem;
    case ax::mojom::Role::kListItem:
        return QAccessible::ListItem;
    case ax::mojom::Role::kListGrid:
        return  QAccessible::List;
    case ax::mojom::Role::kListMarker:
        return QAccessible::StaticText;
    case ax::mojom::Role::kLog:
        return QAccessible::Section;
    case ax::mojom::Role::kMain:
        return QAccessible::Grouping;
    case ax::mojom::Role::kMark:
        return QAccessible::StaticText;
    case ax::mojom::Role::kMarquee:
        return QAccessible::Section;
    case ax::mojom::Role::kMath:
    case ax::mojom::Role::kMathMLMath:
        return QAccessible::Equation;
    case ax::mojom::Role::kMathMLFraction:
        return QAccessible::Grouping;
    case ax::mojom::Role::kMathMLIdentifier:
        return QAccessible::StaticText;
    case ax::mojom::Role::kMathMLMultiscripts:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLNoneScript:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLNumber:
        return QAccessible::StaticText;
    case ax::mojom::Role::kMathMLOperator:
        return QAccessible::StaticText;
    case ax::mojom::Role::kMathMLOver:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLPrescriptDelimiter:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLRoot:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLRow:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLSquareRoot:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLStringLiteral:
        return QAccessible::StaticText;
    case ax::mojom::Role::kMathMLSub:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLSubSup:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLSup:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLTable:
        return QAccessible::Table;
    case ax::mojom::Role::kMathMLTableCell:
        return QAccessible::Cell;
    case ax::mojom::Role::kMathMLTableRow:
        return QAccessible::Row;
    case ax::mojom::Role::kMathMLText:
        return QAccessible::StaticText;
    case ax::mojom::Role::kMathMLUnder:
        return QAccessible::Section;
    case ax::mojom::Role::kMathMLUnderOver:
        return QAccessible::Section;
    case ax::mojom::Role::kMenu:
        return QAccessible::PopupMenu;
    case ax::mojom::Role::kMenuBar:
        return QAccessible::MenuBar;
    case ax::mojom::Role::kMenuItem:
        return QAccessible::MenuItem;
    case ax::mojom::Role::kMenuItemCheckBox:
        return QAccessible::CheckBox;
    case ax::mojom::Role::kMenuItemRadio:
        return QAccessible::RadioButton;
    case ax::mojom::Role::kMenuListOption:
        return QAccessible::MenuItem;
    case ax::mojom::Role::kMenuListPopup:
        return QAccessible::PopupMenu;
    case ax::mojom::Role::kMeter:
        return QAccessible::Chart;
    case ax::mojom::Role::kNavigation:
        return QAccessible::Section;
    case ax::mojom::Role::kNote:
        return QAccessible::Note;
    case ax::mojom::Role::kPane:
        return QAccessible::Pane;
    case ax::mojom::Role::kParagraph:
        return QAccessible::Paragraph;
    case ax::mojom::Role::kPdfActionableHighlight:
        return QAccessible::Button;
    case ax::mojom::Role::kPdfRoot:
        return QAccessible::Document;
    case ax::mojom::Role::kPluginObject:
        return QAccessible::Grouping;
    case ax::mojom::Role::kPopUpButton:
        return QAccessible::ComboBox;
    case ax::mojom::Role::kPortal:
        return QAccessible::Button;
    case ax::mojom::Role::kPre:
        return QAccessible::Section;
    case ax::mojom::Role::kProgressIndicator:
        return QAccessible::ProgressBar;
    case ax::mojom::Role::kRadioButton:
        return QAccessible::RadioButton;
    case ax::mojom::Role::kRadioGroup:
        return QAccessible::Grouping;
    case ax::mojom::Role::kRegion:
        return QAccessible::Section;
    case ax::mojom::Role::kRow:
        return QAccessible::Row;
    case ax::mojom::Role::kRowGroup:
        return QAccessible::Section;
    case ax::mojom::Role::kRowHeader:
        return QAccessible::RowHeader;
    case ax::mojom::Role::kRuby:
        return QAccessible::Grouping;
    case ax::mojom::Role::kRubyAnnotation:
        return QAccessible::StaticText;
    case ax::mojom::Role::kScrollBar:
        return QAccessible::ScrollBar;
    case ax::mojom::Role::kScrollView:
        return QAccessible::Pane;
    case ax::mojom::Role::kSearch:
        return QAccessible::Section;
    case ax::mojom::Role::kSearchBox:
        return QAccessible::EditableText;
    case ax::mojom::Role::kSection:
        return QAccessible::Section;
    case ax::mojom::Role::kSlider:
        return QAccessible::Slider;
    case ax::mojom::Role::kSpinButton:
        return QAccessible::SpinBox;
    case ax::mojom::Role::kSplitter:
        return QAccessible::Splitter;
    case ax::mojom::Role::kStaticText:
        return QAccessible::StaticText;
    case ax::mojom::Role::kStatus:
        return QAccessible::Indicator;
    case ax::mojom::Role::kStrong:
        return QAccessible::StaticText;
    case ax::mojom::Role::kSubscript:
        return QAccessible::Grouping;
    case ax::mojom::Role::kSuggestion:
        return QAccessible::Section;
    case ax::mojom::Role::kSuperscript:
        return QAccessible::Grouping;
    case ax::mojom::Role::kSvgRoot:
        return QAccessible::WebDocument;
    case ax::mojom::Role::kSwitch:
        return QAccessible::Button;
    case ax::mojom::Role::kTable:
        return QAccessible::Table;
    case ax::mojom::Role::kTableHeaderContainer:
        return QAccessible::Section;
    case ax::mojom::Role::kTab:
        return QAccessible::PageTab;
    case ax::mojom::Role::kTabList:
        return QAccessible::PageTabList;
    case ax::mojom::Role::kTabPanel:
        return QAccessible::Pane;
    case ax::mojom::Role::kTerm:
        return QAccessible::StaticText;
    case ax::mojom::Role::kTextField:
        return QAccessible::EditableText;
    case ax::mojom::Role::kTime:
    case ax::mojom::Role::kTimer:
        return QAccessible::Clock;
    case ax::mojom::Role::kTitleBar:
        return QAccessible::Document;
    case ax::mojom::Role::kToggleButton:
        return QAccessible::Button;
    case ax::mojom::Role::kToolbar:
        return QAccessible::ToolBar;
    case ax::mojom::Role::kTooltip:
        return QAccessible::ToolTip;
    case ax::mojom::Role::kTree:
        return QAccessible::Tree;
    case ax::mojom::Role::kTreeGrid:
        return QAccessible::Tree;
    case ax::mojom::Role::kTreeItem:
        return QAccessible::TreeItem;
    case ax::mojom::Role::kVideo:
        return QAccessible::Animation;
    case ax::mojom::Role::kWindow:
        return QAccessible::Window;
    }
    return QAccessible::NoRole;
}

QAccessible::State BrowserAccessibilityInterface::state() const
{
    QAccessible::State state = QAccessible::State();
    if (!q->isReady()) {
        state.invalid = true;
        return state;
    }

    if (q->HasState(ax::mojom::State::kCollapsed))
        state.collapsed = true;
    if (q->HasState(ax::mojom::State::kDefault))
        state.defaultButton = true;
    if (q->HasState(ax::mojom::State::kEditable))
        state.editable = true;
    if (q->HasState(ax::mojom::State::kExpanded))
        state.expanded = true;
    if (q->HasState(ax::mojom::State::kFocusable))
        state.focusable = true;
    if (q->HasState(ax::mojom::State::kHorizontal))
    {} // FIXME
    if (q->HasState(ax::mojom::State::kHovered))
        state.hotTracked = true;
    if (q->HasState(ax::mojom::State::kIgnored))
    {} // FIXME
    if (q->HasState(ax::mojom::State::kInvisible))
        state.invisible = true;
    if (q->HasState(ax::mojom::State::kLinked))
        state.linked = true;
    if (q->HasState(ax::mojom::State::kMultiline))
        state.multiLine = true;
    if (q->HasState(ax::mojom::State::kMultiselectable))
        state.multiSelectable = true;
    if (q->HasState(ax::mojom::State::kProtected))
        state.passwordEdit = true;
    if (q->HasState(ax::mojom::State::kRequired))
    {} // FIXME
    if (q->HasState(ax::mojom::State::kRichlyEditable))
    {} // FIXME
    if (q->HasState(ax::mojom::State::kVertical))
    {} // FIXME
    if (q->HasState(ax::mojom::State::kVisited))
        state.traversed = true;

    if (q->IsOffscreen())
        state.offscreen = true;
    if (q->manager()->GetFocus() == q)
        state.focused = true;
    if (q->GetBoolAttribute(ax::mojom::BoolAttribute::kBusy))
        state.busy = true;
    if (q->GetBoolAttribute(ax::mojom::BoolAttribute::kModal))
        state.modal = true;
    if (q->HasBoolAttribute(ax::mojom::BoolAttribute::kSelected)) {
        state.selectable = true;
        state.selected = q->GetBoolAttribute(ax::mojom::BoolAttribute::kSelected);
    }
    if (q->HasIntAttribute(ax::mojom::IntAttribute::kCheckedState)) {
        state.checkable = true;
        const ax::mojom::CheckedState checkedState =
                static_cast<ax::mojom::CheckedState>(q->GetIntAttribute(ax::mojom::IntAttribute::kCheckedState));
        switch (checkedState) {
        case ax::mojom::CheckedState::kTrue:
            if (q->GetRole() == ax::mojom::Role::kToggleButton)
                state.pressed = true;
            else
                state.checked = true;
            break;
        case ax::mojom::CheckedState::kMixed:
            state.checkStateMixed = true;
            break;
        case ax::mojom::CheckedState::kFalse:
        case ax::mojom::CheckedState::kNone:
            break;
        }
    }
    if (q->HasIntAttribute(ax::mojom::IntAttribute::kRestriction)) {
        const ax::mojom::Restriction restriction = static_cast<ax::mojom::Restriction>(q->GetIntAttribute(ax::mojom::IntAttribute::kRestriction));
        switch (restriction) {
        case ax::mojom::Restriction::kReadOnly:
            state.readOnly = true;
            break;
        case ax::mojom::Restriction::kDisabled:
            state.disabled = true;
            break;
        case ax::mojom::Restriction::kNone:
            break;
        }
    }
    if (q->HasIntAttribute(ax::mojom::IntAttribute::kHasPopup)) {
        const ax::mojom::HasPopup hasPopup = static_cast<ax::mojom::HasPopup>(q->GetIntAttribute(ax::mojom::IntAttribute::kHasPopup));
        switch (hasPopup) {
        case ax::mojom::HasPopup::kFalse:
            break;
        case ax::mojom::HasPopup::kTrue:
        case ax::mojom::HasPopup::kMenu:
        case ax::mojom::HasPopup::kListbox:
        case ax::mojom::HasPopup::kTree:
        case ax::mojom::HasPopup::kGrid:
        case ax::mojom::HasPopup::kDialog:
            state.hasPopup = true;
            break;
        }
    }
    return state;
}

QStringList BrowserAccessibilityInterface::actionNames() const
{
    QStringList actions;
    if (q->HasState(ax::mojom::State::kFocusable))
        actions << QAccessibleActionInterface::setFocusAction();
    return actions;
}

void BrowserAccessibilityInterface::doAction(const QString &actionName)
{
    if (actionName == QAccessibleActionInterface::setFocusAction())
        q->manager()->SetFocus(*q);
}

QStringList
BrowserAccessibilityInterface::keyBindingsForAction(const QString & /*actionName*/) const
{
    QT_NOT_YET_IMPLEMENTED
    return QStringList();
}

void BrowserAccessibilityInterface::addSelection(int startOffset, int endOffset)
{
    q->manager()->SetSelection(content::BrowserAccessibility::AXRange(q->CreatePositionAt(startOffset), q->CreatePositionAt(endOffset)));
}

QString BrowserAccessibilityInterface::attributes(int offset, int *startOffset, int *endOffset) const
{
    *startOffset = offset;
    *endOffset = offset;
    return QString();
}

int BrowserAccessibilityInterface::cursorPosition() const
{
    int pos = 0;
    q->GetIntAttribute(ax::mojom::IntAttribute::kTextSelStart, &pos);
    return pos;
}

QRect BrowserAccessibilityInterface::characterRect(int /*offset*/) const
{
    QT_NOT_YET_IMPLEMENTED
    return QRect();
}

int BrowserAccessibilityInterface::selectionCount() const
{
    int start = 0;
    int end = 0;
    q->GetIntAttribute(ax::mojom::IntAttribute::kTextSelStart, &start);
    q->GetIntAttribute(ax::mojom::IntAttribute::kTextSelEnd, &end);
    if (start != end)
        return 1;
    return 0;
}

int BrowserAccessibilityInterface::offsetAtPoint(const QPoint &/*point*/) const
{
    QT_NOT_YET_IMPLEMENTED
    return 0;
}

void BrowserAccessibilityInterface::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
    Q_ASSERT(startOffset && endOffset);
    *startOffset = 0;
    *endOffset = 0;
    if (selectionIndex != 0)
        return;
    q->GetIntAttribute(ax::mojom::IntAttribute::kTextSelStart, startOffset);
    q->GetIntAttribute(ax::mojom::IntAttribute::kTextSelEnd, endOffset);
}

QString BrowserAccessibilityInterface::text(int startOffset, int endOffset) const
{
    return text(QAccessible::Value).mid(startOffset, endOffset - startOffset);
}

void BrowserAccessibilityInterface::removeSelection(int selectionIndex)
{
    q->manager()->SetSelection(content::BrowserAccessibility::AXRange(q->CreatePositionAt(0), q->CreatePositionAt(0)));
}

void BrowserAccessibilityInterface::setCursorPosition(int position)
{
    q->manager()->SetSelection(content::BrowserAccessibility::AXRange(q->CreatePositionAt(position), q->CreatePositionAt(position)));
}

void BrowserAccessibilityInterface::setSelection(int selectionIndex, int startOffset, int endOffset)
{
    if (selectionIndex != 0)
        return;
    q->manager()->SetSelection(content::BrowserAccessibility::AXRange(q->CreatePositionAt(startOffset), q->CreatePositionAt(endOffset)));
}

int BrowserAccessibilityInterface::characterCount() const
{
    return text(QAccessible::Value).length();
}

void BrowserAccessibilityInterface::scrollToSubstring(int startIndex, int endIndex)
{
    int count = characterCount();
    if (startIndex < endIndex && endIndex < count)
        q->manager()->ScrollToMakeVisible(*q,
                                          q->GetRootFrameHypertextRangeBoundsRect(
                                              startIndex,
                                              endIndex - startIndex,
                                              ui::AXClippingBehavior::kUnclipped));
}

QVariant BrowserAccessibilityInterface::currentValue() const
{
    QVariant result;
    float value;
    if (q->GetFloatAttribute(ax::mojom::FloatAttribute::kValueForRange, &value)) {
        result = (double) value;
    }
    return result;
}

void BrowserAccessibilityInterface::setCurrentValue(const QVariant &value)
{
    // not yet implemented anywhere in blink
    QT_NOT_YET_IMPLEMENTED
}

QVariant BrowserAccessibilityInterface::maximumValue() const
{
    QVariant result;
    float value;
    if (q->GetFloatAttribute(ax::mojom::FloatAttribute::kMaxValueForRange, &value)) {
        result = (double) value;
    }
    return result;
}

QVariant BrowserAccessibilityInterface::minimumValue() const
{
    QVariant result;
    float value;
    if (q->GetFloatAttribute(ax::mojom::FloatAttribute::kMinValueForRange, &value)) {
        result = (double) value;
    }
    return result;
}

QVariant BrowserAccessibilityInterface::minimumStepSize() const
{
    QVariant result;
    float value;
    if (q->GetFloatAttribute(ax::mojom::FloatAttribute::kStepValueForRange, &value)) {
        result = (double) value;
    }
    return result;
}

QAccessibleInterface *BrowserAccessibilityInterface::cellAt(int row, int column) const
{
    int columns = 0;
    int rows = 0;
    if (!q->GetIntAttribute(ax::mojom::IntAttribute::kTableColumnCount, &columns)
        || !q->GetIntAttribute(ax::mojom::IntAttribute::kTableRowCount, &rows)
        || columns <= 0
        || rows <= 0) {
        return nullptr;
    }

    if (row < 0 || row >= rows || column < 0 || column >= columns)
        return nullptr;

    absl::optional<int> cell_id = q->GetCellId(row, column);
    content::BrowserAccessibility *cell = cell_id ? q->manager()->GetFromID(*cell_id) : nullptr;
    if (cell)
        return content::toQAccessibleInterface(cell);

    return nullptr;
}

QAccessibleInterface *BrowserAccessibilityInterface::caption() const
{
    return nullptr;
}

QAccessibleInterface *BrowserAccessibilityInterface::summary() const
{
    return nullptr;
}

QString BrowserAccessibilityInterface::columnDescription(int column) const
{
    return QString();
}

QString BrowserAccessibilityInterface::rowDescription(int row) const
{
    return QString();
}

int BrowserAccessibilityInterface::columnCount() const
{
    int columns = 0;
    if (q->GetIntAttribute(ax::mojom::IntAttribute::kTableColumnCount, &columns))
        return columns;
    return 0;
}

int BrowserAccessibilityInterface::rowCount() const
{
    int rows = 0;
    if (q->GetIntAttribute(ax::mojom::IntAttribute::kTableRowCount, &rows))
        return rows;
    return 0;
}

int BrowserAccessibilityInterface::selectedCellCount() const
{
    return 0;
}

int BrowserAccessibilityInterface::selectedColumnCount() const
{
    return 0;
}

int BrowserAccessibilityInterface::selectedRowCount() const
{
    return 0;
}

QList<QAccessibleInterface *> BrowserAccessibilityInterface::selectedCells() const
{
    return QList<QAccessibleInterface *>();
}

QList<int> BrowserAccessibilityInterface::selectedColumns() const
{
    return QList<int>();
}

QList<int> BrowserAccessibilityInterface::selectedRows() const
{
    return QList<int>();
}

bool BrowserAccessibilityInterface::isColumnSelected(int /*column*/) const
{
    return false;
}

bool BrowserAccessibilityInterface::isRowSelected(int /*row*/) const
{
    return false;
}

bool BrowserAccessibilityInterface::selectRow(int /*row*/)
{
    return false;
}

bool BrowserAccessibilityInterface::selectColumn(int /*column*/)
{
    return false;
}

bool BrowserAccessibilityInterface::unselectRow(int /*row*/)
{
    return false;
}

bool BrowserAccessibilityInterface::unselectColumn(int /*column*/)
{
    return false;
}

int BrowserAccessibilityInterface::columnExtent() const
{
    return 1;
}

QList<QAccessibleInterface *> BrowserAccessibilityInterface::columnHeaderCells() const
{
    return QList<QAccessibleInterface*>();
}

int BrowserAccessibilityInterface::columnIndex() const
{
    int column = 0;
    if (q->GetIntAttribute(ax::mojom::IntAttribute::kTableCellColumnIndex, &column))
        return column;
    return 0;
}

int BrowserAccessibilityInterface::rowExtent() const
{
    return 1;
}

QList<QAccessibleInterface *> BrowserAccessibilityInterface::rowHeaderCells() const
{
    return QList<QAccessibleInterface*>();
}

int BrowserAccessibilityInterface::rowIndex() const
{
    int row = 0;
    if (q->GetIntAttribute(ax::mojom::IntAttribute::kTableCellRowIndex, &row))
        return row;
    return 0;
}

bool BrowserAccessibilityInterface::isSelected() const
{
    return false;
}

content::BrowserAccessibility *BrowserAccessibilityInterface::findTable() const
{
    content::BrowserAccessibility *parent = q->PlatformGetParent();
    while (parent && parent->GetRole() != ax::mojom::Role::kTable)
        parent = parent->PlatformGetParent();

    return parent;
}

QAccessibleInterface *BrowserAccessibilityInterface::table() const
{
    content::BrowserAccessibility *table = findTable();
    Q_ASSERT(table);
    return content::toQAccessibleInterface(table);
}

void BrowserAccessibilityInterface::modelChange(QAccessibleTableModelChangeEvent *)
{
}

} // namespace QtWebEngineCore

namespace content {

// static
std::unique_ptr<BrowserAccessibility> BrowserAccessibility::Create(BrowserAccessibilityManager *man, ui::AXNode *node)
{
    return std::unique_ptr<BrowserAccessibility>(new QtWebEngineCore::BrowserAccessibilityQt(man, node));
}

QAccessibleInterface *toQAccessibleInterface(BrowserAccessibility *obj)
{
    return static_cast<QtWebEngineCore::BrowserAccessibilityQt *>(obj)->interface;
}

const QAccessibleInterface *toQAccessibleInterface(const BrowserAccessibility *obj)
{
    return static_cast<const QtWebEngineCore::BrowserAccessibilityQt *>(obj)->interface;
}

} // namespace content
