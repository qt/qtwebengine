// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginepermission.h"
#include "qwebenginepermission_p.h"
#include "web_contents_adapter.h"
#include "profile_adapter.h"

QT_BEGIN_NAMESPACE

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QWebEnginePermissionPrivate)

QWebEnginePermissionPrivate::QWebEnginePermissionPrivate()
    : QSharedData()
    , feature(QWebEnginePermission::Unsupported)
{
}

QWebEnginePermissionPrivate::QWebEnginePermissionPrivate(const QUrl &origin_, QWebEnginePermission::Feature feature_,
        QSharedPointer<QtWebEngineCore::WebContentsAdapter> webContentsAdapter_, QtWebEngineCore::ProfileAdapter *profileAdapter_)
    : QSharedData()
    , origin(origin_)
    , feature(feature_)
    , webContentsAdapter(webContentsAdapter_)
    , profileAdapter(profileAdapter_)
{
}

/*!
    \class QWebEnginePermission
    \inmodule QtWebEngineCore
    \since 6.8
    \brief A QWebEnginePermission is an object used to access and modify the state of a single permission that's been
    granted or denied to a specific origin URL.

    The typical usage pattern is as follows:
    \list 1
        \li A website requests a specific feature, triggering the QWebEnginePage::permissionRequested() signal;
        \li The signal handler triggers a prompt asking the user whether they want to grant the permission;
        \li When the user has made their decision, the application calls \l grant() or \l deny();
    \endlist

    Alternatively, an application interested in modifying already granted permissions may use QWebEngineProfile::listPermissions()
    to get a list of existing permissions associated with a profile, or QWebEngineProfile::getPermission() to get
    a QWebEnginePermission object for a specific permission.

    The \l origin() property can be used to query which origin the QWebEnginePermission is associated with, while the
    \l feature() property describes the associated feature. A website origin is the combination of its scheme, hostname,
    and port. Permissions are granted on a per-origin basis; thus, if the web page \c{https://www.example.com:12345/some/page.html}
    requests a permission, it will be granted to the origin \c{https://www.example.com:12345/}.

    \l QWebEnginePermission::Feature describes all the feature types Qt WebEngine supports. Some Features are transient;
    in practice, this means that they are never remembered, and a website that uses them will trigger a permission
    prompt every time the Feature is needed. Transient Features cannot be granted in advance.

    The usability lifetime of a QWebEnginePermission is tied either to its associated QWebEnginePage
    (for transient feature types), or QWebEngineProfile (for permanent feature types). A transient permission is one which
    needs to be explicitly granted or denied every time it's needed (e.g. webcam/screen sharing permission), whereas a permanent
    one might be stored inside the current profile, depending on the value of QWebEngineProfile::persistentPermissionsPolicy().
    You can check whether a QWebEnginePermission is in a valid state using its \l isValid() property. For invalid objects, calls to \l grant(),
    \l deny(), or \l reset() will do nothing, while calls to \l state() will always return QWebEnginePermission::Invalid.

    \sa QWebEnginePage::permissionRequested(), QWebEngineProfile::getPermission(), QWebEngineProfile::listPermissions()
*/

/*! \fn QWebEnginePermission::QWebEnginePermission()
    \internal
*/

/*! \internal */
QWebEnginePermission::QWebEnginePermission()
    : d_ptr(new QWebEnginePermissionPrivate())
{
}

/*! \internal */
QWebEnginePermission::QWebEnginePermission(QWebEnginePermissionPrivate *pvt)
    : d_ptr(pvt)
{
}

QWebEnginePermission::QWebEnginePermission(const QWebEnginePermission &other)
    : d_ptr(new QWebEnginePermissionPrivate(*other.d_ptr))
{
}

QWebEnginePermission::~QWebEnginePermission() = default;

QWebEnginePermission &QWebEnginePermission::operator=(const QWebEnginePermission &other)
{
    if (this == &other)
        return *this;

    d_ptr.reset(new QWebEnginePermissionPrivate(*other.d_ptr));
    return *this;
}

bool QWebEnginePermission::comparesEqual(const QWebEnginePermission &other) const
{
    if (this == &other)
        return true;

    if (d_ptr->feature != other.d_ptr->feature || d_ptr->origin != other.d_ptr->origin)
        return false;

    if (isTransient(d_ptr->feature)) {
        if (d_ptr->webContentsAdapter != other.d_ptr->webContentsAdapter)
            return false;
    } else {
        QtWebEngineCore::ProfileAdapter *thisProfile = d_ptr->webContentsAdapter
            ? d_ptr->webContentsAdapter.toStrongRef()->profileAdapter()
            : d_ptr->profileAdapter.get();
        QtWebEngineCore::ProfileAdapter *otherProfile = d_ptr->webContentsAdapter
            ? other.d_ptr->webContentsAdapter.toStrongRef()->profileAdapter()
            : other.d_ptr->profileAdapter.get();

        if (thisProfile != otherProfile)
            return false;
    }

    return true;
}

/*!
    \property QWebEnginePermission::origin
    \brief The URL of the permission's associated origin.

    A website origin is the combination of its scheme, hostname, and port. Permissions are granted on a
    per-origin basis; thus, if the web page \c{https://www.example.com:12345/some/page.html}
    requests a permission, it will be granted to the origin \c{https://www.example.com:12345/}.
*/
const QUrl QWebEnginePermission::origin() const
{
    return d_ptr->origin;
}

/*!
    \enum QWebEnginePermission::Feature

    This enum type holds the type of the requested feature:

    \value MediaAudioCapture Access to a microphone, or another audio source. This feature is transient.
    \value MediaVideoCapture Access to a webcam, or another video source. This feature is transient.
    \value MediaAudioVideoCapture Combination of \l MediaAudioCapture and \l MediaVideoCapture. This feature is transient.
    \value DesktopVideoCapture Access to the contents of the user's screen. This feature is transient.
    \value DesktopAudioVideoCapture Access to the contents of the user's screen, and application audio. This feature is transient.
    \value MouseLock Locks the pointer inside an element on the web page. This feature is transient.
    \value Notifications Allows the website to send notifications to the user.
    \value Geolocation Access to the user's physical location.
    \value ClipboardReadWrite Access to the user's clipboard.
    \value LocalFontsAccess Access to the fonts installed on the user's machine. Only available on desktops.
    \value Unsupported An unsupported feature type.

    \note Transient feature types are ones that will never be remembered by the underlying storage, and will trigger
    a permission request every time a website tries to use them. Transient Features can only be denied/granted
    as they're needed; any attempts to pre-grant a transient Feature will fail.
*/

/*!
    \property QWebEnginePermission::feature
    \brief The feature type associated with this permission.
*/
QWebEnginePermission::Feature QWebEnginePermission::feature() const
{
    return d_ptr->feature;
}

/*!
    \enum QWebEnginePermission::State

    This enum type holds the current state of the requested feature:

    \value Invalid Object is in an invalid state, and any attempts to modify the described permission will fail.
    \value Ask Either the permission has not been requested before, or the feature() is transient.
    \value Granted Permission has already been granted.
    \value Denied Permission has already been denied.
*/

/*!
    \property QWebEnginePermission::state
    \brief The current state of the permission.

    If a permission for the specified \l feature() and \l origin() has already been granted or denied,
    the return value is QWebEnginePermission::Granted, or QWebEnginePermission::Denied, respectively.
    When this is the first time the permission is requested, or if the \l feature() is transient,
    the return value is QWebEnginePermission::Ask. If the object is in an invalid state, the returned
    value is QWebEnginePermission::Invalid.

    \sa isValid(), isTransient()
*/
QWebEnginePermission::State QWebEnginePermission::state() const
{
    if (!isValid())
        return Invalid;
    if (d_ptr->webContentsAdapter)
        return d_ptr->webContentsAdapter.toStrongRef()->getPermissionState(origin(), feature());
    if (d_ptr->profileAdapter)
        return d_ptr->profileAdapter->getPermissionState(origin(), feature());
    Q_UNREACHABLE();
    return Ask;
}

/*!
    \property QWebEnginePermission::isValid
    \brief Indicates whether attempts to change the permission's state will be successful.

    An invalid QWebEnginePermission is either:
    \list
        \li One whose \l feature() is unsupported;
        \li One whose \l feature() is transient, and the associated page/view has been destroyed;
        \li One whose \l feature() is permanent, but the associated profile has been destroyed;
        \li One whose \l origin() is invalid.
    \endlist

    \sa isTransient()
*/
bool QWebEnginePermission::isValid() const
{
    if (feature() == Unsupported)
        return false;
    if (isTransient(feature()) && !d_ptr->webContentsAdapter)
        return false;
    if (!d_ptr->profileAdapter)
        return false;
    if (!d_ptr->origin.isValid())
        return false;
    return true;
}

/*!
    Allows the associated origin to access the requested feature. Does nothing when \l isValid() evaluates to false.

    \sa deny(), reset(), isValid()
*/
void QWebEnginePermission::grant() const
{
    if (!isValid())
        return;
    if (d_ptr->webContentsAdapter)
        d_ptr->webContentsAdapter.toStrongRef()->setFeaturePermission(origin(), feature(), Granted);
    else if (d_ptr->profileAdapter)
        d_ptr->profileAdapter->setPermission(origin(), feature(), Granted);
}

/*!
    Stops the associated origin from accessing the requested feature. Does nothing when \l isValid() evaluates to false.

    \sa grant(), reset(), isValid()
*/
void QWebEnginePermission::deny() const
{
    if (!isValid())
        return;
    if (d_ptr->webContentsAdapter)
        d_ptr->webContentsAdapter.toStrongRef()->setFeaturePermission(origin(), feature(), Denied);
    else if (d_ptr->profileAdapter)
        d_ptr->profileAdapter->setPermission(origin(), feature(), Denied);
}

/*!
    Removes the permission from the profile's underlying storage. By default, permissions are stored on disk (except for
    off-the-record profiles, where permissions are stored in memory and are destroyed with the profile).
    This means that an already granted/denied permission will not be requested twice, but will get automatically
    granted/denied every subsequent time a website requests it. Calling reset() allows the query to be asked
    again the next time the website requests it.

    Does nothing when \l isValid() evaluates to false.

    \sa grant(), deny(), isValid(), QWebEngineProfile::persistentPermissionsPolicy()
*/
void QWebEnginePermission::reset() const
{
    if (!isValid())
        return;
    if (d_ptr->webContentsAdapter)
        d_ptr->webContentsAdapter.toStrongRef()->setFeaturePermission(origin(), feature(), Ask);
    else if (d_ptr->profileAdapter)
        d_ptr->profileAdapter->setPermission(origin(), feature(), Ask);
}

/*!
    Returns whether \a feature is transient, meaning that a permission will be requested
    every time the associated functionality is used by a web page.
*/
bool QWebEnginePermission::isTransient(QWebEnginePermission::Feature feature)
{
    switch (feature) {
    case QWebEnginePermission::MediaAudioCapture:
    case QWebEnginePermission::MediaVideoCapture:
    case QWebEnginePermission::MediaAudioVideoCapture:
    case QWebEnginePermission::DesktopVideoCapture:
    case QWebEnginePermission::DesktopAudioVideoCapture:
    case QWebEnginePermission::MouseLock:
        return true;
    case QWebEnginePermission::Notifications:
    case QWebEnginePermission::Geolocation:
    case QWebEnginePermission::ClipboardReadWrite:
    case QWebEnginePermission::LocalFontsAccess:
        return false;
    case QWebEnginePermission::Unsupported:
        return false;
    }
}

QT_END_NAMESPACE

#include "moc_qwebenginepermission.cpp"
