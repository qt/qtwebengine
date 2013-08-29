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

#include "qquickwebengineview_p.h"
#include "qquickwebengineview_p_p.h"

#include "web_contents_adapter.h"
#include "render_widget_host_view_qt_delegate_quick.h"

#include <QAbstractListModel>
#include <QFileInfo>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlProperty>
#include <QUrl>

#include <private/qqmlmetatype_p.h>

// Uncomment for QML debugging
#define UI_DELEGATES_DEBUG

class MenuItem : public QObject {
    Q_OBJECT
public:

    MenuItem(const QString& text, const QString iconName = QString(), QObject *parent = 0)
        : QObject(parent)
        , m_text(text)
        , m_iconName(iconName)
        , m_enabled(true)
    {
    }

    inline QString text() const { return m_text; }
    inline QString iconName() const { return m_iconName; }
    inline bool enabled() const { return m_enabled; }
    inline void setEnabled(bool on) { m_enabled = on; }

Q_SIGNALS:
    void triggered();

private:
    QString m_text;
    QString m_iconName;
    bool m_enabled;
};


#include "qquickwebengineview.moc"

QT_BEGIN_NAMESPACE

QQuickWebEngineViewPrivate::QQuickWebEngineViewPrivate()
    : adapter(new WebContentsAdapter)
    , loadProgress(0)
    , contextMenuExtraItems(0)
    , menuComponent(0)
    , menuItemComponent(0)
    , menuSeparatorComponent(0)
{
    adapter->initialize(this);
}

RenderWidgetHostViewQtDelegate *QQuickWebEngineViewPrivate::CreateRenderWidgetHostViewQtDelegate(CompositingMode)
{
    return new RenderWidgetHostViewQtDelegateQuickPainted;
}

void QQuickWebEngineViewPrivate::titleChanged(const QString &title)
{
    Q_Q(QQuickWebEngineView);
    Q_UNUSED(title);
    Q_EMIT q->titleChanged();
}

void QQuickWebEngineViewPrivate::urlChanged(const QUrl &url)
{
    Q_Q(QQuickWebEngineView);
    Q_UNUSED(url);
    Q_EMIT q->urlChanged();
}

void QQuickWebEngineViewPrivate::iconChanged(const QUrl &url)
{
    Q_Q(QQuickWebEngineView);
    icon = url;
    Q_EMIT q->iconChanged();
}

void QQuickWebEngineViewPrivate::loadingStateChanged()
{
    Q_Q(QQuickWebEngineView);
    Q_EMIT q->loadingStateChanged();
}

void QQuickWebEngineViewPrivate::loadProgressChanged(int progress)
{
    Q_Q(QQuickWebEngineView);
    loadProgress = progress;
    Q_EMIT q->loadProgressChanged();
}

QRectF QQuickWebEngineViewPrivate::viewportRect() const
{
    Q_Q(const QQuickWebEngineView);
    return QRectF(q->x(), q->y(), q->width(), q->height());
}

void QQuickWebEngineViewPrivate::loadFinished(bool success)
{
    Q_Q(QQuickWebEngineView);
    Q_UNUSED(success);
    Q_EMIT q->loadingStateChanged();
}

void QQuickWebEngineViewPrivate::focusContainer()
{
    Q_Q(QQuickWebEngineView);
    q->forceActiveFocus();
}

void QQuickWebEngineViewPrivate::adoptNewWindow(WebContentsAdapter *newWebContents, WindowOpenDisposition disposition)
{
    Q_UNUSED(newWebContents);
    Q_UNUSED(disposition);
    Q_UNREACHABLE();
}

bool QQuickWebEngineViewPrivate::ensureComponentLoaded(QQmlComponent *&component, const QString& fileName)
{
#ifndef UI_DELEGATES_DEBUG
    if (component)
        return true;
#endif
    component = loadDefaultUIDelegate(fileName);

    if (component->status() != QQmlComponent::Ready) {
#ifdef UI_DELEGATES_DEBUG
        Q_FOREACH (const QQmlError& err, component->errors())
            fprintf(stderr, "  component error: %s\n", qPrintable(err.toString()));
#endif
        return false;
    }
    return true;
}


QQmlContext *QQuickWebEngineViewPrivate::creationContextForComponent(QQmlComponent *component)
{
    Q_ASSERT(component);
    Q_Q(QQuickWebEngineView);

    QQmlContext* baseContext = component->creationContext();
    if (!baseContext)
        baseContext = new QQmlContext(qmlEngine(q)->rootContext());
    return baseContext;
}

static void initFromMenuItem(QObject* qmlItem, MenuItem* item) {
    Q_ASSERT(item);
    QQmlProperty prop(qmlItem, QStringLiteral("text"));
    prop.write(item->text());
    prop = QQmlProperty(qmlItem, QStringLiteral("iconName"));
    prop.write(item->iconName());
    prop = QQmlProperty(qmlItem, QStringLiteral("enabled"));
    prop.write(item->enabled());
    prop = QQmlProperty(qmlItem, QStringLiteral("onTriggered"));
#ifdef UI_DELEGATES_DEBUG
    if (!prop.isSignalProperty())
        fprintf(stderr, "MenuItem is missing onTriggered signal property\n");
#endif
    QObject::connect(qmlItem, prop.method(), item, QMetaMethod::fromSignal(&MenuItem::triggered));

}


void QQuickWebEngineViewPrivate::addMenuItem(QObject *menu, MenuItem *menuItem)
{
    Q_Q(QQuickWebEngineView);
    if (!ensureComponentLoaded(menuItemComponent, QStringLiteral("MenuItem.qml")))
        return;

    QObject* it = menuItemComponent->create(creationContextForComponent(menuItemComponent));
    initFromMenuItem(it, menuItem);
    menuItem->setParent(it); // cleanup purposes

    it->setParent(menu);

    QQmlListReference entries(menu, QQmlMetaType::defaultProperty(menu).name(), qmlEngine(q));
    if (entries.isValid())
        entries.append(it);
}

void QQuickWebEngineViewPrivate::addMenuSeparator(QObject *menu)
{
    Q_Q(QQuickWebEngineView);
    if (!ensureComponentLoaded(menuSeparatorComponent, QStringLiteral("MenuSeparator.qml")))
        return;

    QQmlContext *itemContext = creationContextForComponent(menuSeparatorComponent);
    QObject* sep = menuSeparatorComponent->create(itemContext);
    sep->setParent(menu);

    QQmlListReference entries(menu, QQmlMetaType::defaultProperty(menu).name(), qmlEngine(q));
    if (entries.isValid())
        entries.append(sep);
}

QObject *QQuickWebEngineViewPrivate::addMenu(QObject *parentMenu, const QString &title, const QPoint& pos)
{
    Q_Q(QQuickWebEngineView);
    if (!ensureComponentLoaded(menuComponent, QStringLiteral("Menu.qml")))
        return 0;
    QQmlContext *context(creationContextForComponent(menuComponent));
    QObject *menu = menuComponent->beginCreate(context);
    // Useful when not using Qt Quick Controls' Menu
    if (QQuickItem* item = qobject_cast<QQuickItem*>(menu))
        item->setParentItem(q);
    menuComponent->completeCreate();

    if (!title.isEmpty()) {
        QQmlProperty titleProp(menu, QStringLiteral("title"));
        titleProp.write(title);
    }
    if (!pos.isNull()) {
        QQmlProperty posProp(menu, QStringLiteral("pos"));
        posProp.write(pos);
    }
    if (!parentMenu) {
        QQmlProperty doneSignal(menu, QStringLiteral("onDone"));
        static int deleteLaterIndex = menu->metaObject()->indexOfSlot("deleteLater()");
        if (doneSignal.isSignalProperty())
            QObject::connect(menu, doneSignal.method(), menu, menu->metaObject()->method(deleteLaterIndex));
    } else {
        menu->setParent(parentMenu);

        QQmlListReference entries(parentMenu, QQmlMetaType::defaultProperty(parentMenu).name(), qmlEngine(q));
        if (entries.isValid())
            entries.append(menu);
    }
    return menu;
}

QQmlComponent *QQuickWebEngineViewPrivate::loadDefaultUIDelegate(const QString &fileName)
{
    Q_Q(QQuickWebEngineView);
    QQmlEngine* engine = qmlEngine(q);
    if (!engine)
        return new QQmlComponent(q);
    QString absolutePath;
    Q_FOREACH (const QString &path, engine->importPathList()) {
        QFileInfo fi(path + QStringLiteral("/QtWebEngine/UIDelegates/") + fileName);
        if (fi.exists())
            absolutePath = fi.absoluteFilePath();
    }

    return new QQmlComponent(engine, QUrl(absolutePath), QQmlComponent::PreferSynchronous, q);
}

bool QQuickWebEngineViewPrivate::contextMenuRequested(const WebEngineContextMenuData &data)
{
    Q_Q(QQuickWebEngineView);

    QObject *menu = addMenu(0, QString(), data.pos);
    if (!menu)
        return false;

    // Populate our menu
    MenuItem *item = 0;

    item = new MenuItem(QObject::tr("Back"), QStringLiteral("go-previous"));
    QObject::connect(item, &MenuItem::triggered, q, &QQuickWebEngineView::goBack);
    item->setEnabled(q->canGoBack());
    addMenuItem(menu, item);


    item = new MenuItem(QObject::tr("Forward"), QStringLiteral("go-next"));
    QObject::connect(item, &MenuItem::triggered, q, &QQuickWebEngineView::goForward);
    item->setEnabled(q->canGoForward());
    addMenuItem(menu, item);

    item = new MenuItem(QObject::tr("Reload"), QStringLiteral("view-refresh"));
    QObject::connect(item, &MenuItem::triggered, q, &QQuickWebEngineView::reload);
    addMenuItem(menu, item);

    if (contextMenuExtraItems) {
        addMenuSeparator(menu);
        if (QObject* menuExtras = contextMenuExtraItems->create(creationContextForComponent(contextMenuExtraItems))) {
            menuExtras->setParent(menu);
            QQmlListReference entries(menu, QQmlMetaType::defaultProperty(menu).name(), qmlEngine(q));
            if (entries.isValid())
                entries.append(menuExtras);
        }
    }

    // Now fire the popup() method on the top level menu
    QMetaObject::invokeMethod(menu, "popup");
    return true;
}

QQuickWebEngineView::QQuickWebEngineView(QQuickItem *parent)
    : QQuickItem(*(new QQuickWebEngineViewPrivate), parent)
{
}

QQuickWebEngineView::~QQuickWebEngineView()
{
}

QUrl QQuickWebEngineView::url() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->activeUrl();
}

void QQuickWebEngineView::setUrl(const QUrl& url)
{
    Q_D(QQuickWebEngineView);
    d->adapter->load(url);
}

QUrl QQuickWebEngineView::icon() const
{
    Q_D(const QQuickWebEngineView);
    return d->icon;
}

void QQuickWebEngineView::goBack()
{
    Q_D(QQuickWebEngineView);
    d->adapter->navigateToOffset(-1);
}

void QQuickWebEngineView::goForward()
{
    Q_D(QQuickWebEngineView);
    d->adapter->navigateToOffset(1);
}

void QQuickWebEngineView::reload()
{
    Q_D(QQuickWebEngineView);
    d->adapter->reload();
}

void QQuickWebEngineView::stop()
{
    Q_D(QQuickWebEngineView);
    d->adapter->stop();
}

bool QQuickWebEngineView::isLoading() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->isLoading();
}

int QQuickWebEngineView::loadProgress() const
{
    Q_D(const QQuickWebEngineView);
    return d->loadProgress;
}

QString QQuickWebEngineView::title() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->pageTitle();
}

bool QQuickWebEngineView::canGoBack() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->canGoBack();
}

bool QQuickWebEngineView::canGoForward() const
{
    Q_D(const QQuickWebEngineView);
    return d->adapter->canGoForward();
}

void QQuickWebEngineView::setContextMenuExtraItems(QQmlComponent *contextMenu)
{
    Q_D(QQuickWebEngineView);
    if (d->contextMenuExtraItems == contextMenu)
        return;
    d->contextMenuExtraItems = contextMenu;
    emit contextMenuExtraItemsChanged();
}

QQmlComponent *QQuickWebEngineView::contextMenuExtraItems() const
{
    Q_D(const QQuickWebEngineView);
    return d->contextMenuExtraItems;
}

void QQuickWebEngineView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    Q_FOREACH(QQuickItem *child, childItems()) {
        Q_ASSERT(qobject_cast<RenderWidgetHostViewQtDelegateQuickPainted *>(child));
        child->setSize(newGeometry.size());
    }
}

QT_END_NAMESPACE
