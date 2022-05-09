/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebenginefilesystemaccessrequest.h"

#include "file_system_access/file_system_access_permission_request_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineFileSystemAccessRequest
    \brief The QWebEngineFileSystemAccessRequest class enables accepting or rejecting
     requests for local file system access from JavaScript applications.

    \since 6.4

    \inmodule QtWebEngineCore

    To allow web applications to access local files of the computer,
    applications must connect to QWebEnginePage::fileSystemAccessRequested, which takes a
    QWebEngineFileSystemAccessRequest instance as an argument.

    If a web applications requests access to local files or directories,
    QWebEnginePage::fileSystemAccessRequested will be emitted with an
    QWebEngineFileSystemAccessRequest instance as an argument where accessFlags() indicates
    the type of the requested access: read, write or both. The signal handler needs to then
    either call accept() or reject().
*/

QWebEngineFileSystemAccessRequest::QWebEngineFileSystemAccessRequest(
        const QWebEngineFileSystemAccessRequest &other) = default;
QWebEngineFileSystemAccessRequest &QWebEngineFileSystemAccessRequest::operator=(
        const QWebEngineFileSystemAccessRequest &other) = default;
QWebEngineFileSystemAccessRequest::QWebEngineFileSystemAccessRequest(
        QWebEngineFileSystemAccessRequest &&other) = default;
QWebEngineFileSystemAccessRequest &
QWebEngineFileSystemAccessRequest::operator=(QWebEngineFileSystemAccessRequest &&other) = default;
QWebEngineFileSystemAccessRequest::~QWebEngineFileSystemAccessRequest() = default;

/*! \fn bool QWebEngineFileSystemAccessRequest::operator==(const QWebEngineFileSystemAccessRequest &that) const
    Returns \c true if \a that points to the same object as this request.
*/
bool QWebEngineFileSystemAccessRequest::operator==(
        const QWebEngineFileSystemAccessRequest &that) const
{
    return d_ptr == that.d_ptr;
}

/*! \fn bool QWebEngineFileSystemAccessRequest::operator!=(const QWebEngineFileSystemAccessRequest &that) const
    Returns \c true if \a that points to a different object than this request.
*/
bool QWebEngineFileSystemAccessRequest::operator!=(
        const QWebEngineFileSystemAccessRequest &that) const
{
    return d_ptr != that.d_ptr;
}

/*! \internal */
QWebEngineFileSystemAccessRequest::QWebEngineFileSystemAccessRequest(
        QSharedPointer<QtWebEngineCore::FileSystemAccessPermissionRequestController> controller)
    : d_ptr(controller)
{
}

/*!
    Rejects a request to access local files.
*/
void QWebEngineFileSystemAccessRequest::reject()
{
    d_ptr->reject();
}

/*!
    Accepts the request to access local files.
*/
void QWebEngineFileSystemAccessRequest::accept()
{
    d_ptr->accept();
}

/*!
    \property QWebEngineFileSystemAccessRequest::origin
    \brief The URL of the web page that issued the file system access request.
*/

QUrl QWebEngineFileSystemAccessRequest::origin() const
{
    return d_ptr->origin();
}

/*!
    \property QWebEngineFileSystemAccessRequest::filePath
    \brief Returns the file path this file system access request is referring to.
*/

QUrl QWebEngineFileSystemAccessRequest::filePath() const
{
    return d_ptr->filePath();
}

/*!
    \property QWebEngineFileSystemAccessRequest::handleType
    \brief Returns the type of the requested file system entry. (File or directory)
 */
HandleType QWebEngineFileSystemAccessRequest::handleType() const
{
    return d_ptr->handleType();
}

/*!
    \property QWebEngineFileSystemAccessRequest::accessFlags
    \brief Contains the requested file access rights.
 */
AccessFlags QWebEngineFileSystemAccessRequest::accessFlags() const
{
    return d_ptr->accessFlags();
}

QT_END_NAMESPACE
