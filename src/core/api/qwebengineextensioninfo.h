// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEEXTENSIONINFO_H_
#define QWEBENGINEEXTENSIONINFO_H_

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#if QT_CONFIG(webengine_extensions)
#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include <QtQml/qqmlregistration.h>

namespace QtWebEngineCore {
class ExtensionManager;
}

QT_BEGIN_NAMESPACE
class QWebEngineExtensionInfoPrivate;
class QWebEngineExtensionManager;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR(QWebEngineExtensionInfoPrivate)

class QWebEngineExtensionInfo
{
    Q_GADGET_EXPORT(Q_WEBENGINECORE_EXPORT)
    Q_PROPERTY(QString name READ name FINAL)
    Q_PROPERTY(QString id READ id FINAL)
    Q_PROPERTY(QString description READ description FINAL)
    Q_PROPERTY(QString path READ path FINAL)
    Q_PROPERTY(QString error READ error FINAL)
    Q_PROPERTY(QUrl actionPopupUrl READ actionPopupUrl FINAL)
    Q_PROPERTY(bool isEnabled READ isEnabled FINAL)
    Q_PROPERTY(bool isLoaded READ isLoaded FINAL)
    Q_PROPERTY(bool isInstalled READ isInstalled FINAL)

public:
    QML_VALUE_TYPE(webEngineExtension)
    QML_ADDED_IN_VERSION(6, 10)

    Q_WEBENGINECORE_EXPORT QWebEngineExtensionInfo();

    Q_WEBENGINECORE_EXPORT
    QWebEngineExtensionInfo(const QWebEngineExtensionInfo &other) noexcept;
    QWebEngineExtensionInfo(QWebEngineExtensionInfo &&other) noexcept = default;
    Q_WEBENGINECORE_EXPORT
    QWebEngineExtensionInfo &operator=(const QWebEngineExtensionInfo &other) noexcept;
    Q_WEBENGINECORE_EXPORT ~QWebEngineExtensionInfo();

    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QWebEngineExtensionInfo)
    void swap(QWebEngineExtensionInfo &other) noexcept { d_ptr.swap(other.d_ptr); }

    Q_WEBENGINECORE_EXPORT QString name() const;
    Q_WEBENGINECORE_EXPORT QString id() const;
    Q_WEBENGINECORE_EXPORT QString description() const;
    Q_WEBENGINECORE_EXPORT QString path() const;
    Q_WEBENGINECORE_EXPORT QString error() const;
    Q_WEBENGINECORE_EXPORT QUrl actionPopupUrl() const;
    Q_WEBENGINECORE_EXPORT bool isEnabled() const;
    Q_WEBENGINECORE_EXPORT bool isLoaded() const;
    Q_WEBENGINECORE_EXPORT bool isInstalled() const;

private:
    friend class QtWebEngineCore::ExtensionManager;
    friend class QWebEngineExtensionManager;

    Q_WEBENGINECORE_EXPORT
    QWebEngineExtensionInfo(QWebEngineExtensionInfoPrivate *d);

    QExplicitlySharedDataPointer<QWebEngineExtensionInfoPrivate> d_ptr;
};

Q_DECLARE_SHARED(QWebEngineExtensionInfo)
QT_END_NAMESPACE

#endif // QT_CONFIG(webengine_extensions)
#endif // QWEBENGINEEXTENSIONINFO_H_
