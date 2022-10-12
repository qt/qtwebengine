// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "ui/accessibility/platform/inspect/ax_tree_formatter_base.h"

#include <utility>

#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "content/browser/accessibility/accessibility_tree_formatter_blink.h"
#include "content/browser/accessibility/browser_accessibility.h"
#include "content/public/browser/ax_inspect_factory.h"
#include "ui/accessibility/platform/inspect/ax_event_recorder.h"

#include "browser_accessibility_qt.h"

#include <QtGui/qaccessible.h>

namespace content {

#if QT_CONFIG(accessibility)
class AccessibilityTreeFormatterQt : public ui::AXTreeFormatterBase {
public:
    explicit AccessibilityTreeFormatterQt();
    ~AccessibilityTreeFormatterQt() override;

   base::Value BuildTree(ui::AXPlatformNodeDelegate *start) const override;
   base::Value BuildTreeForSelector(const AXTreeSelector &selector) const override
   {
       return base::Value{};
   }

private:
    void RecursiveBuildAccessibilityTree(const BrowserAccessibility &node, base::Value::Dict *dict) const;
    void AddProperties(const BrowserAccessibility &node, base::Value::Dict *dict) const;
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
    base::Value::Dict dict;
    RecursiveBuildAccessibilityTree(*root_internal, &dict);
    return base::Value(std::move(dict));
}

void AccessibilityTreeFormatterQt::RecursiveBuildAccessibilityTree(const BrowserAccessibility &node, base::Value::Dict *dict) const
{
    AddProperties(node, dict);

    base::Value::List children;
    for (size_t i = 0; i < node.PlatformChildCount(); ++i) {
        base::Value::Dict child_dict;

        content::BrowserAccessibility *child_node = node.PlatformGetChild(i);

        RecursiveBuildAccessibilityTree(*child_node, &child_dict);
        children.Append(std::move(child_dict));
    }
    dict->Set(kChildrenDictAttr, std::move(children));
}

void AccessibilityTreeFormatterQt::AddProperties(const BrowserAccessibility &node, base::Value::Dict *dict) const
{
    dict->Set("id", node.GetId());
    const QAccessibleInterface *iface = toQAccessibleInterface(&node);

    dict->Set("role", qAccessibleRoleString(iface->role()));

    QAccessible::State state = iface->state();

    base::Value::List states;
    if (state.busy)
        states.Append(base::Value("busy"));
    if (state.checkable)
        states.Append(base::Value("checkable"));
    if (state.checked)
        states.Append(base::Value("checked"));
    if (node.IsClickable())
        states.Append(base::Value("clickable"));
    if (state.collapsed)
        states.Append(base::Value("collapsed"));
    if (state.disabled)
        states.Append(base::Value("disabled"));
    if (state.editable)
        states.Append(base::Value("editable"));
    if (state.expandable)
        states.Append(base::Value("expandable"));
    if (state.expanded)
        states.Append(base::Value("expanded"));
    if (state.focusable)
        states.Append(base::Value("focusable"));
    if (state.focused)
        states.Append(base::Value("focused"));
    if (state.hasPopup)
        states.Append(base::Value("hasPopup"));
    if (state.hotTracked)
        states.Append(base::Value("hotTracked"));
    if (state.invisible)
        states.Append(base::Value("invisible"));
    if (state.linked)
        states.Append(base::Value("linked"));
    if (state.multiLine)
        states.Append(base::Value("multiLine"));
    if (state.multiSelectable)
        states.Append(base::Value("multiSelectable"));
    if (state.modal)
        states.Append(base::Value("modal"));
    if (state.offscreen)
        states.Append(base::Value("offscreen"));
    if (state.passwordEdit)
        states.Append(base::Value("password"));
    if (state.pressed)
        states.Append(base::Value("pressed"));
    if (state.readOnly)
        states.Append(base::Value("readOnly"));
    if (state.selectable)
        states.Append(base::Value("selectable"));
    if (state.selected)
        states.Append(base::Value("selected"));
    if (state.traversed)
        states.Append(base::Value("traversed"));
    dict->Set("states", std::move(states));

    dict->Set("name", iface->text(QAccessible::Name).toStdString());
    dict->Set("description", iface->text(QAccessible::Description).toStdString());
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
    if (node.GetList("states", &states_value)) {
        for (const auto &state : states_value->GetList()) {
            if (auto *state_value = state.GetIfString())
                WriteAttribute(false, *state_value, &line);
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
    return AXInspectFactory::CreateFormatter(ui::AXApiType::kQt);
}

// static
std::unique_ptr<ui::AXEventRecorder> AXInspectFactory::CreatePlatformRecorder(BrowserAccessibilityManager *manager,
                                                                              base::ProcessId pid,
                                                                              const ui::AXTreeSelector &selector)
{
    return AXInspectFactory::CreateRecorder(ui::AXApiType::kQt, manager, pid, selector);
}

// static
std::unique_ptr<ui::AXTreeFormatter> AXInspectFactory::CreateFormatter(ui::AXApiType::Type type)
{
    switch (type) {
    case ui::AXApiType::kBlink:
        return std::make_unique<AccessibilityTreeFormatterBlink>();
    case ui::AXApiType::kQt:
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
std::unique_ptr<ui::AXEventRecorder> AXInspectFactory::CreateRecorder(ui::AXApiType::Type type,
                                                                      BrowserAccessibilityManager *manager,
                                                                      base::ProcessId pid,
                                                                      const ui::AXTreeSelector &selector)
{
    switch (type) {
    case ui::AXApiType::kQt:
        return std::make_unique<ui::AXEventRecorder>();
    default:
        NOTREACHED() << "Unsupported inspect type " << type;
    }
    return nullptr;
}

} // namespace content
