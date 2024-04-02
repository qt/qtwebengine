// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEPERMISSION_H
#define QWEBENGINEPERMISSION_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

struct QWebEnginePermissionPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QWebEnginePermissionPrivate,
                                                 Q_WEBENGINECORE_EXPORT)

class QWebEnginePermission
{
    Q_GADGET_EXPORT(Q_WEBENGINECORE_EXPORT)
    Q_PROPERTY(QUrl origin READ origin CONSTANT FINAL)
    Q_PROPERTY(Feature feature READ feature CONSTANT FINAL)
    Q_PROPERTY(State state READ state CONSTANT FINAL)
    Q_PROPERTY(bool isValid READ isValid CONSTANT FINAL)
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

public:
    Q_WEBENGINECORE_EXPORT QWebEnginePermission();

    Q_WEBENGINECORE_EXPORT QWebEnginePermission(const QWebEnginePermission &other);
    Q_WEBENGINECORE_EXPORT QWebEnginePermission(QWebEnginePermissionPrivate *pvt);
    Q_WEBENGINECORE_EXPORT ~QWebEnginePermission();

    Q_WEBENGINECORE_EXPORT QWebEnginePermission &operator=(const QWebEnginePermission &other);
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QWebEnginePermission)
    void swap(QWebEnginePermission &other) noexcept { d_ptr.swap(other.d_ptr); }

    enum Feature : quint8 {
        Unsupported,
        MediaAudioCapture,
        MediaVideoCapture,
        MediaAudioVideoCapture,
        DesktopVideoCapture,
        DesktopAudioVideoCapture,
        MouseLock,
        Notifications,
        Geolocation,
        ClipboardReadWrite,
        LocalFontsAccess,
    };
    Q_ENUM(Feature)

    enum State : quint8 {
        Invalid,
        Ask,
        Granted,
        Denied
    };
    Q_ENUM(State)

    Q_WEBENGINECORE_EXPORT const QUrl origin() const;
    Q_WEBENGINECORE_EXPORT Feature feature() const;
    Q_WEBENGINECORE_EXPORT State state() const;
    Q_WEBENGINECORE_EXPORT bool isValid() const;

    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void grant() const;
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void deny() const;
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void reset() const;

    Q_WEBENGINECORE_EXPORT Q_INVOKABLE static bool isTransient(QWebEnginePermission::Feature feature);

private:
    inline friend bool operator==(const QWebEnginePermission &lhs, const QWebEnginePermission &rhs)
        { return lhs.comparesEqual(rhs); };
    inline friend bool operator!=(const QWebEnginePermission &lhs, const QWebEnginePermission &rhs)
        { return !lhs.comparesEqual(rhs); };

    Q_WEBENGINECORE_EXPORT bool comparesEqual(const QWebEnginePermission &other) const;

protected:
    QExplicitlySharedDataPointer<QWebEnginePermissionPrivate> d_ptr;
};



Q_DECLARE_SHARED(QWebEnginePermission)

QT_END_NAMESPACE

#endif // QWEBENGINEPERMISSION_H
