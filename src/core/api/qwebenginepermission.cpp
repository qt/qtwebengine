// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginepermission.h"
#include "qwebenginepermission_p.h"
#include "web_contents_adapter.h"
#include "profile_adapter.h"

QT_BEGIN_NAMESPACE

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QWebEnginePermissionPrivate)

/*! \internal */
QWebEnginePermissionPrivate::QWebEnginePermissionPrivate()
    : QSharedData()
    , permissionType(QWebEnginePermission::PermissionType::Unsupported)
{
}

/*! \internal */
QWebEnginePermissionPrivate::QWebEnginePermissionPrivate(const QUrl &origin_, QWebEnginePermission::PermissionType permissionType_,
        QSharedPointer<QtWebEngineCore::WebContentsAdapter> webContentsAdapter_, QtWebEngineCore::ProfileAdapter *profileAdapter_)
    : QSharedData()
    , origin(origin_)
    , permissionType(permissionType_)
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
        \li A website requests a specific permission, triggering the QWebEnginePage::permissionRequested() signal;
        \li The signal handler triggers a prompt asking the user whether they want to grant the permission;
        \li When the user has made their decision, the application calls \l grant() or \l deny();
    \endlist

    Alternatively, an application interested in modifying already granted permissions may use QWebEngineProfile::listAllPermissions()
    to get a list of existing permissions associated with a profile, or QWebEngineProfile::queryPermission() to get
    a QWebEnginePermission object for a specific permission.

    The \l origin() property can be used to query which origin the QWebEnginePermission is associated with, while the
    \l permissionType() property describes the type of the requested permission. A website origin is the combination of
    its scheme, hostname, and port. Permissions are granted on a per-origin basis; thus, if the web page
    \c{https://www.example.com:12345/some/page.html} requests a permission, it will be granted to the origin
    \c{https://www.example.com:12345/}.

    \l QWebEnginePermission::PermissionType describes all the permission types Qt WebEngine supports. Only some permission types
    are remembered between browsing sessions; they are \e persistent. Non-persistent permissions query the user every time a
    website requests them. You can check whether a permission type is persistent at runtime
    using the static method QWebEnginePermission::isPersistent().

    Persistent permissions are stored inside the active QWebEngineProfile, and their lifetime depends on the value of
    QWebEngineProfile::persistentPermissionsPolicy(). By default, named profiles store their permissions on disk, whereas
    off-the-record ones store them in memory (and destroy them when the profile is destroyed). A stored permission will not
    query the user the next time a website requests it; instead it will be automatically granted or denied, depending on
    the resolution the user picked initially. To erase a stored permission, call \l reset() on it.

    A non-persistent permission, on the other hand, is only usable until the related QWebEnginePage performs a navigation to
    a different URL, or is destroyed.

    You can check whether a QWebEnginePermission is in a valid state using its \l isValid() property. For invalid objects, calls to \l grant(),
    \l deny(), or \l reset() will do nothing, while calls to \l state() will always return QWebEnginePermission::Invalid.

    \sa QWebEnginePage::permissionRequested(), QWebEngineProfile::queryPermission(), QWebEngineProfile::listAllPermissions()
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
    : d_ptr(other.d_ptr)
{
}

QWebEnginePermission::~QWebEnginePermission() = default;

QWebEnginePermission &QWebEnginePermission::operator=(const QWebEnginePermission &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

bool QWebEnginePermission::equals(const QWebEnginePermission &other) const
{
    if (this == &other)
        return true;

    if (!d_ptr || !other.d_ptr)
        return false;

    if (d_ptr->permissionType != other.d_ptr->permissionType || d_ptr->origin != other.d_ptr->origin)
        return false;

    if (!isPersistent(d_ptr->permissionType)) {
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
QUrl QWebEnginePermission::origin() const
{
    return d_ptr ? d_ptr->origin : QUrl();
}

/*!
    \enum QWebEnginePermission::PermissionType

    This enum type holds the type of the requested permission type:

    \value MediaAudioCapture Access to a microphone, or another audio source. This permission is \e not persistent.
    \value MediaVideoCapture Access to a webcam, or another video source. This permission is \e not persistent.
    \value MediaAudioVideoCapture Combination of \l MediaAudioCapture and \l MediaVideoCapture. This permission is \e not persistent.
    \value DesktopVideoCapture Access to the contents of the user's screen. This permission is \e not persistent.
    \value DesktopAudioVideoCapture Access to the contents of the user's screen, and application audio. This permission is \e not persistent.
    \value MouseLock Locks the pointer inside an element on the web page. This permission is \e not persistent.
    \value Notifications Allows the website to send notifications to the user. This permission is persistent.
    \value Geolocation Access to the user's physical location. This permission is persistent.
    \value ClipboardReadWrite Access to the user's clipboard. This permission is persistent.
    \value LocalFontsAccess Access to the fonts installed on the user's machine. Only available on desktops. This permission is persistent.
    \value Unsupported An unsupported permission type.

    \note Non-persistent permission types are ones that will never be remembered by the underlying storage, and will trigger
    a permission request every time a website tries to use them.
*/

/*!
    \property QWebEnginePermission::permissionType
    \brief The permission type associated with this permission.
*/
QWebEnginePermission::PermissionType QWebEnginePermission::permissionType() const
{
    return d_ptr ? d_ptr->permissionType : PermissionType::Unsupported;
}

/*!
    \enum QWebEnginePermission::State

    This enum type holds the current state of the requested permission:

    \value Invalid Object is in an invalid state, and any attempts to modify the described permission will fail.
    \value Ask Either the permission has not been requested before, or the permissionType() is not persistent.
    \value Granted Permission has already been granted.
    \value Denied Permission has already been denied.
*/

/*!
    \property QWebEnginePermission::state
    \brief The current state of the permission.

    If a permission for the specified \l permissionType() and \l origin() has already been granted or denied,
    the return value is QWebEnginePermission::Granted, or QWebEnginePermission::Denied, respectively.
    When this is the first time the permission is requested,
    the return value is QWebEnginePermission::Ask. If the object is in an invalid state, the returned
    value is QWebEnginePermission::Invalid.

    \sa isValid(), isPersistent()
*/
QWebEnginePermission::State QWebEnginePermission::state() const
{
    if (!isValid())
        return State::Invalid;
    if (d_ptr->webContentsAdapter)
        return d_ptr->webContentsAdapter.toStrongRef()->getPermissionState(origin(), permissionType());
    if (d_ptr->profileAdapter)
        return d_ptr->profileAdapter->getPermissionState(origin(), permissionType());
    Q_UNREACHABLE_RETURN(State::Ask);
}

/*!
    \property QWebEnginePermission::isValid
    \brief Indicates whether attempts to change the permission's state will be successful.

    An invalid QWebEnginePermission is either:
    \list
        \li One whose \l permissionType() is unsupported;
        \li One whose \l origin() is invalid;
        \li One whose associated profile has been destroyed
    \endlist

    \sa isPersistent()
*/
bool QWebEnginePermission::isValid() const
{
    if (!d_ptr)
        return false;
    if (permissionType() == PermissionType::Unsupported)
        return false;
    if (!d_ptr->profileAdapter && !d_ptr->webContentsAdapter)
        return false;
    if (!d_ptr->origin.isValid())
        return false;
    return true;
}

/*!
    Allows the associated origin to access the requested permissionType. Does nothing when \l isValid() evaluates to false.

    \sa deny(), reset(), isValid()
*/
void QWebEnginePermission::grant() const
{
    if (!isValid())
        return;
    if (d_ptr->webContentsAdapter)
        d_ptr->webContentsAdapter.toStrongRef()->setPermission(origin(), permissionType(), State::Granted);
    else if (d_ptr->profileAdapter)
        d_ptr->profileAdapter->setPermission(origin(), permissionType(), State::Granted);
}

/*!
    Stops the associated origin from accessing the requested permissionType. Does nothing when \l isValid() evaluates to false.

    \sa grant(), reset(), isValid()
*/
void QWebEnginePermission::deny() const
{
    if (!isValid())
        return;
    if (d_ptr->webContentsAdapter)
        d_ptr->webContentsAdapter.toStrongRef()->setPermission(origin(), permissionType(), State::Denied);
    else if (d_ptr->profileAdapter)
        d_ptr->profileAdapter->setPermission(origin(), permissionType(), State::Denied);
}

/*!
    Removes the permission from the profile's underlying storage. By default, permissions are stored on disk (except for
    off-the-record profiles, where permissions are stored in memory and are destroyed with the profile).
    This means that an already granted/denied permission will not be requested twice, but will get automatically
    granted/denied every subsequent time a website requests it. Calling reset() allows the query to be displayed
    again the next time the website requests it.

    Does nothing when \l isValid() evaluates to false.

    \sa grant(), deny(), isValid(), QWebEngineProfile::persistentPermissionsPolicy()
*/
void QWebEnginePermission::reset() const
{
    if (!isValid())
        return;
    if (d_ptr->webContentsAdapter)
        d_ptr->webContentsAdapter.toStrongRef()->setPermission(origin(), permissionType(), State::Ask);
    else if (d_ptr->profileAdapter)
        d_ptr->profileAdapter->setPermission(origin(), permissionType(), State::Ask);
}

/*!
    Returns whether a \a permissionType is \e persistent, meaning that a permission's state will be remembered
    and the user will not be queried the next time the website requests the same permission.
*/
bool QWebEnginePermission::isPersistent(QWebEnginePermission::PermissionType permissionType)
{
    switch (permissionType) {
    case QWebEnginePermission::PermissionType::Notifications:
    case QWebEnginePermission::PermissionType::Geolocation:
    case QWebEnginePermission::PermissionType::ClipboardReadWrite:
    case QWebEnginePermission::PermissionType::LocalFontsAccess:
        return true;
    case QWebEnginePermission::PermissionType::MediaAudioCapture:
    case QWebEnginePermission::PermissionType::MediaVideoCapture:
    case QWebEnginePermission::PermissionType::MediaAudioVideoCapture:
    case QWebEnginePermission::PermissionType::DesktopVideoCapture:
    case QWebEnginePermission::PermissionType::DesktopAudioVideoCapture:
    case QWebEnginePermission::PermissionType::MouseLock:
        return false;
    case QWebEnginePermission::PermissionType::Unsupported:
        return false;
    }

    Q_UNREACHABLE_RETURN(false);
}

QT_END_NAMESPACE

#include "moc_qwebenginepermission.cpp"
