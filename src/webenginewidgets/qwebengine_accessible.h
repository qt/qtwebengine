// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINE_ACCESSIBLE_H
#define QWEBENGINE_ACCESSIBLE_H

#include <QtCore/QPointer>
#include <QtWidgets/QAccessibleWidget>

QT_BEGIN_NAMESPACE
class QWebEngineView;

class QWebEngineViewAccessible : public QAccessibleWidget
{
public:
    QWebEngineViewAccessible(QWebEngineView *o);

    bool isValid() const override;
    QAccessibleInterface *focusChild() const override;
    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *child) const override;

private:
    QWebEngineView *view() const;
};

QT_END_NAMESPACE

namespace QtWebEngineCore {

class RenderWidgetHostViewQtDelegateWidgetAccessible : public QAccessibleWidget
{
public:
    RenderWidgetHostViewQtDelegateWidgetAccessible(QWidget *o, QWebEngineView *view);

    bool isValid() const override;
    QAccessibleInterface *focusChild() const override;
    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *child) const override;

private:
    QWebEngineViewAccessible *viewAccessible() const;
    QPointer<QWebEngineView> m_view;
};
} // namespace QtWebEngineCore

#endif // QWEBENGINE_ACCESSIBLE_H
