/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "ui/accessibility/platform/inspect/ax_tree_formatter_base.h"

#include <utility>

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "content/browser/accessibility/accessibility_event_recorder.h"
#include "content/browser/accessibility/accessibility_tree_formatter_blink.h"
#include "content/public/browser/ax_inspect_factory.h"

#include "browser_accessibility_qt.h"

namespace content {

#if QT_CONFIG(accessibility)
class AccessibilityTreeFormatterQt : public ui::AXTreeFormatterBase {
public:
    explicit AccessibilityTreeFormatterQt();
    ~AccessibilityTreeFormatterQt() override;

   base::Value BuildTree(ui::AXPlatformNodeDelegate *start) const override;
   base::Value BuildTreeForWindow(gfx::AcceleratedWidget hwnd) const override
   {
       return base::Value{};
   }
   base::Value BuildTreeForSelector(const AXTreeSelector &selector) const override
   {
       return base::Value{};
   }

private:
    void RecursiveBuildAccessibilityTree(const BrowserAccessibility &node, base::DictionaryValue *dict) const;
    void AddProperties(const BrowserAccessibility &node, base::DictionaryValue *dict) const;
    std::string ProcessTreeForOutput(const base::DictionaryValue &node) const override;
};

AccessibilityTreeFormatterQt::AccessibilityTreeFormatterQt()
{
}

AccessibilityTreeFormatterQt::~AccessibilityTreeFormatterQt()
{
}

base::Value AccessibilityTreeFormatterQt::BuildTree(ui::AXPlatformNodeDelegate *start) const
{
    BrowserAccessibility *root_internal =
        BrowserAccessibility::FromAXPlatformNodeDelegate(start);
    base::Value dict(base::Value::Type::DICTIONARY);
    RecursiveBuildAccessibilityTree(*root_internal, static_cast<base::DictionaryValue *>(&dict));
    return dict;
}

void AccessibilityTreeFormatterQt::RecursiveBuildAccessibilityTree(const BrowserAccessibility &node, base::DictionaryValue *dict) const
{
    AddProperties(node, dict);

    auto children = std::make_unique<base::ListValue>();
    for (size_t i = 0; i < node.PlatformChildCount(); ++i) {
        std::unique_ptr<base::DictionaryValue> child_dict(new base::DictionaryValue);

        content::BrowserAccessibility *child_node = node.PlatformGetChild(i);

        RecursiveBuildAccessibilityTree(*child_node, child_dict.get());
        children->Append(std::move(child_dict));
    }
    dict->Set(kChildrenDictAttr, std::move(children));
}

void AccessibilityTreeFormatterQt::AddProperties(const BrowserAccessibility &node, base::DictionaryValue *dict) const
{
    dict->SetInteger("id", node.GetId());
    const BrowserAccessibilityQt *acc_node = ToBrowserAccessibilityQt(&node);

    dict->SetString("role", qAccessibleRoleString(acc_node->role()));

    QAccessible::State state = acc_node->state();

    std::vector<base::Value> states;
    if (state.busy)
        states.push_back(base::Value("busy"));
    if (state.checkable)
        states.push_back(base::Value("checkable"));
    if (state.checked)
        states.push_back(base::Value("checked"));
    if (acc_node->IsClickable())
        states.push_back(base::Value("clickable"));
    if (state.collapsed)
        states.push_back(base::Value("collapsed"));
    if (state.disabled)
        states.push_back(base::Value("disabled"));
    if (state.editable)
        states.push_back(base::Value("editable"));
    if (state.expandable)
        states.push_back(base::Value("expandable"));
    if (state.expanded)
        states.push_back(base::Value("expanded"));
    if (state.focusable)
        states.push_back(base::Value("focusable"));
    if (state.focused)
        states.push_back(base::Value("focused"));
    if (state.hasPopup)
        states.push_back(base::Value("hasPopup"));
    if (state.hotTracked)
        states.push_back(base::Value("hotTracked"));
    if (state.invisible)
        states.push_back(base::Value("invisible"));
    if (state.linked)
        states.push_back(base::Value("linked"));
    if (state.multiLine)
        states.push_back(base::Value("multiLine"));
    if (state.multiSelectable)
        states.push_back(base::Value("multiSelectable"));
    if (state.modal)
        states.push_back(base::Value("modal"));
    if (state.offscreen)
        states.push_back(base::Value("offscreen"));
    if (state.passwordEdit)
        states.push_back(base::Value("password"));
    if (state.pressed)
        states.push_back(base::Value("pressed"));
    if (state.readOnly)
        states.push_back(base::Value("readOnly"));
    if (state.selectable)
        states.push_back(base::Value("selectable"));
    if (state.selected)
        states.push_back(base::Value("selected"));
    if (state.traversed)
        states.push_back(base::Value("traversed"));
    dict->SetKey("states", base::Value(states));

    dict->SetString("name", acc_node->text(QAccessible::Name).toStdString());
    dict->SetString("description", acc_node->text(QAccessible::Description).toStdString());
}

std::string AccessibilityTreeFormatterQt::ProcessTreeForOutput(const base::DictionaryValue &node) const
{
    std::string error_value;
    if (node.GetString("error", &error_value))
        return error_value;

    std::string line;
    std::string role_value;
    node.GetString("role", &role_value);
    if (!role_value.empty())
        WriteAttribute(true, base::StringPrintf("%s", role_value.c_str()), &line);

    const base::ListValue *states_value = nullptr;
    node.GetList("states", &states_value);
    if (states_value) {
        for (const auto &state : *states_value) {
            std::string state_value;
            if (state.GetAsString(&state_value))
                WriteAttribute(true, state_value, &line);
        }
    }

    std::string name_value;
    if (node.GetString("name", &name_value))
        WriteAttribute(true, base::StringPrintf("name='%s'", name_value.c_str()), &line);

    std::string description_value;
    if (node.GetString("description", &description_value))
        WriteAttribute(false, base::StringPrintf("description='%s'", description_value.c_str()), &line);

    int id_value;
    node.GetInteger("id", &id_value);
    WriteAttribute(false, base::StringPrintf("id=%d", id_value), &line);

    return line + "\n";
}

#endif // QT_CONFIG(accessibility)

// static
std::unique_ptr<ui::AXTreeFormatter>
AXInspectFactory::CreatePlatformFormatter()
{
    return AXInspectFactory::CreateFormatter(kQt);
}

// static
std::unique_ptr<ui::AXEventRecorder> AXInspectFactory::CreatePlatformRecorder(BrowserAccessibilityManager *manager,
                                                                              base::ProcessId pid,
                                                                              const ui::AXTreeSelector &selector)
{
    return AXInspectFactory::CreateRecorder(kQt, manager, pid, selector);
}

// static
std::unique_ptr<ui::AXTreeFormatter> AXInspectFactory::CreateFormatter(AXInspectFactory::Type type)
{
    switch (type) {
    case kBlink:
        return std::make_unique<AccessibilityTreeFormatterBlink>();
    case kQt:
#if QT_CONFIG(accessibility)
        return std::make_unique<AccessibilityTreeFormatterQt>();
#else
        return nullptr;
#endif
    default:
        NOTREACHED() << "Unsupported inspect type " << type;
    }
    return nullptr;
}

// static
std::unique_ptr<ui::AXEventRecorder> AXInspectFactory::CreateRecorder(AXInspectFactory::Type type,
                                                                      BrowserAccessibilityManager *manager,
                                                                      base::ProcessId pid,
                                                                      const ui::AXTreeSelector &selector)
{
    switch (type) {
    case kQt:
        return std::make_unique<AccessibilityEventRecorder>(manager);
    default:
        NOTREACHED() << "Unsupported inspect type " << type;
    }
    return nullptr;
}

} // namespace content
