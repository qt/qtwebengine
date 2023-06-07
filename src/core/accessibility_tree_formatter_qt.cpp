// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "content/browser/accessibility/browser_accessibility_manager.h"
#include "content/browser/accessibility/accessibility_tree_formatter_blink.h"
#include "content/public/browser/ax_inspect_factory.h"
#include "ui/accessibility/platform/inspect/ax_event_recorder.h"

#include <QtGui/qtguiglobal.h>

#include <memory>
#include <string>
#include <utility>

#if QT_CONFIG(accessibility)
#include "browser_accessibility_qt.h"

#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "content/browser/accessibility/browser_accessibility.h"
#include "ui/accessibility/platform/inspect/ax_tree_formatter_base.h"

#include <QtGui/qaccessible.h>
#endif

namespace content {

#if QT_CONFIG(accessibility)
class AccessibilityTreeFormatterQt : public ui::AXTreeFormatterBase {
public:
    explicit AccessibilityTreeFormatterQt();
    ~AccessibilityTreeFormatterQt() override;

   base::Value::Dict BuildTree(ui::AXPlatformNodeDelegate *start) const override;
   base::Value::Dict BuildTreeForSelector(const AXTreeSelector &selector) const override
   {
       return base::Value::Dict{};
   }

private:
    void RecursiveBuildAccessibilityTree(const BrowserAccessibility &node, base::Value::Dict *dict) const;
    void AddProperties(const BrowserAccessibility &node, base::Value::Dict *dict) const;
    std::string ProcessTreeForOutput(const base::Value::Dict &node) const override;
};

AccessibilityTreeFormatterQt::AccessibilityTreeFormatterQt()
{
}

AccessibilityTreeFormatterQt::~AccessibilityTreeFormatterQt()
{
}

base::Value::Dict AccessibilityTreeFormatterQt::BuildTree(ui::AXPlatformNodeDelegate *start) const
{
    BrowserAccessibility *root_internal =
        BrowserAccessibility::FromAXPlatformNodeDelegate(start);
    base::Value::Dict dict;
    RecursiveBuildAccessibilityTree(*root_internal, &dict);
    return dict;
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

std::string AccessibilityTreeFormatterQt::ProcessTreeForOutput(const base::Value::Dict &node) const
{
    std::string error_value;
    if (auto error_value = node.FindString("error"))
        return *error_value;

    std::string line;
    std::string role_value;
    if (auto role_value = node.FindString("role"))
        WriteAttribute(true, base::StringPrintf("%s", role_value->c_str()), &line);

    if (const auto states_value = node.FindList("states")) {
        for (const auto &state : *states_value) {
            if (auto *state_value = state.GetIfString())
                WriteAttribute(false, *state_value, &line);
        }
    }

    if (auto name_value = node.FindString("name"))
        WriteAttribute(true, base::StringPrintf("name='%s'", name_value->c_str()), &line);

    if (auto description_value = node.FindString("description"))
        WriteAttribute(false, base::StringPrintf("description='%s'", description_value->c_str()), &line);

    int id_value;
    if (auto maybe_id = node.FindInt("id"))
        id_value = *maybe_id;
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
