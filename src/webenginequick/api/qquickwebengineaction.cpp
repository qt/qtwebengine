// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#include "qquickwebengineaction_p.h"
#include "qquickwebengineaction_p_p.h"
#include "qquickwebengineview_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype WebEngineAction
    \instantiates QQuickWebEngineAction
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.8

    \brief An action that represents a \l WebEngineView::WebAction.

    A WebEngineAction is returned by the \l WebEngineView::action()
    method. It provides information about the action, such as
    whether it is \l enabled.

    The following code uses the \l WebEngineView::action() method to check if
    the copy action is enabled:

    \code
    var copyAction = webEngineView.action(WebEngineView.Copy);
    if (copyAction.enabled)
        console.log("Copy is enabled.");
    else
        console.log("Copy is disabled.");
    \endcode

    A \l ToolButton can be connected to a WebEngineAction as follows:

    \snippet qtwebengine_webengineaction.qml 0

    A context menu could be implemented like this:

    \snippet qtwebengine_webengineaction.qml 1
*/

QQuickWebEngineActionPrivate::QQuickWebEngineActionPrivate(const QVariant &data, const QString &text, const QString &iconName, bool enabled)
    : m_data(data)
    , m_text(text)
    , m_iconName(iconName)
    , m_enabled(enabled)
{
}

QQuickWebEngineActionPrivate::~QQuickWebEngineActionPrivate()
{
}

void QQuickWebEngineActionPrivate::setEnabled(bool enabled)
{
    Q_Q(QQuickWebEngineAction);
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    emit q->enabledChanged();
}

QVariant QQuickWebEngineActionPrivate::data() const
{
    return m_data;
}

void QQuickWebEngineActionPrivate::trigger()
{
    Q_Q(QQuickWebEngineAction);
    if (QQuickWebEngineView *view = static_cast<QQuickWebEngineView*>(q->parent())) {
        view->triggerWebAction(static_cast<QQuickWebEngineView::WebAction>(data().toInt()));
    }
}

QQuickWebEngineAction::QQuickWebEngineAction(const QVariant &data, const QString &text, const QString &iconName, bool enabled, QObject *parent)
    : QObject(parent)
    , d_ptr(new QQuickWebEngineActionPrivate(data, text, iconName, enabled))
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineAction::QQuickWebEngineAction(QObject *parent)
    : QObject(parent)
    , d_ptr(new QQuickWebEngineActionPrivate(-1, QStringLiteral(""), QStringLiteral(""), false))
{
    d_ptr->q_ptr = this;
}

QQuickWebEngineAction::~QQuickWebEngineAction()
{
}

/*!
    \qmlproperty int WebEngineAction::text

    This property holds a textual description of the action.
*/
QString QQuickWebEngineAction::text() const
{
    Q_D(const QQuickWebEngineAction);
    return d->m_text;
}

/*!
    \qmlproperty string WebEngineAction::iconName

    This property holds the name of the icon for the action. This name
    can be used to pick the icon from a theme.
*/
QString QQuickWebEngineAction::iconName() const
{
    Q_D(const QQuickWebEngineAction);
    return d->m_iconName;
}

/*!
    \qmlproperty bool WebEngineAction::enabled

    This property holds whether the action is enabled.
*/
bool QQuickWebEngineAction::isEnabled() const
{
    Q_D(const QQuickWebEngineAction);
    return d->m_enabled;
}

/*!
    \qmlmethod void WebEngineAction::trigger()

    Triggers the action.
*/
void QQuickWebEngineAction::trigger()
{
    Q_D(QQuickWebEngineAction);
    if (!isEnabled())
        return;

    d->trigger();
    emit triggered();
}

QT_END_NAMESPACE

#include "moc_qquickwebengineaction_p.cpp"
