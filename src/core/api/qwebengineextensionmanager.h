// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBENGINEEXTENSIONMANAGER_H_
#define QWEBENGINEEXTENSIONMANAGER_H_

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#if QT_CONFIG(webengine_extensions)

#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtCore/qobject.h>
#include <QtWebEngineCore/qwebengineextensioninfo.h>

namespace QtWebEngineCore {
class ExtensionManager;
class ProfileAdapter;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineExtensionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString installPath READ installPath FINAL)
    Q_PROPERTY(QList<QWebEngineExtensionInfo> extensions READ extensions FINAL)
public:
    ~QWebEngineExtensionManager() override;
    Q_INVOKABLE void loadExtension(const QString &path);
    Q_INVOKABLE void installExtension(const QString &path);
    Q_INVOKABLE void unloadExtension(const QWebEngineExtensionInfo &extension);
    Q_INVOKABLE void uninstallExtension(const QWebEngineExtensionInfo &extension);
    Q_INVOKABLE void setExtensionEnabled(const QWebEngineExtensionInfo &extension, bool enabled);

    QString installPath() const;
    QList<QWebEngineExtensionInfo> extensions() const;

Q_SIGNALS:
    void loadFinished(const QWebEngineExtensionInfo &extension);
    void installFinished(const QWebEngineExtensionInfo &extension);
    void unloadFinished(const QWebEngineExtensionInfo &extension);
    void uninstallFinished(const QWebEngineExtensionInfo &extension);

private:
    friend class QtWebEngineCore::ProfileAdapter;
    Q_DISABLE_COPY(QWebEngineExtensionManager)

    explicit QWebEngineExtensionManager(QtWebEngineCore::ExtensionManager *d);
    QtWebEngineCore::ExtensionManager *d_ptr;
};

QT_END_NAMESPACE

#endif // QT_CONFIG(webengine_extensions)
#endif // QWEBENGINEEXTENSIONMANAGER_H_
