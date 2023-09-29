// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebenginedownloadrequest.h"
#include "qwebenginedownloadrequest_p.h"

#include "qwebenginepage.h"

#include "profile_adapter.h"
#include "web_contents_adapter_client.h"

#include <QFileInfo>

QT_BEGIN_NAMESPACE

using QtWebEngineCore::ProfileAdapterClient;

ASSERT_ENUMS_MATCH(ProfileAdapterClient::NoReason, QWebEngineDownloadRequest::NoReason)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileFailed, QWebEngineDownloadRequest::FileFailed)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileAccessDenied, QWebEngineDownloadRequest::FileAccessDenied)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileNoSpace, QWebEngineDownloadRequest::FileNoSpace)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileNameTooLong, QWebEngineDownloadRequest::FileNameTooLong)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileTooLarge, QWebEngineDownloadRequest::FileTooLarge)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileVirusInfected, QWebEngineDownloadRequest::FileVirusInfected)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileTransientError, QWebEngineDownloadRequest::FileTransientError)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileBlocked, QWebEngineDownloadRequest::FileBlocked)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileSecurityCheckFailed, QWebEngineDownloadRequest::FileSecurityCheckFailed)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileTooShort, QWebEngineDownloadRequest::FileTooShort)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileHashMismatch, QWebEngineDownloadRequest::FileHashMismatch)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkFailed, QWebEngineDownloadRequest::NetworkFailed)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkTimeout, QWebEngineDownloadRequest::NetworkTimeout)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkDisconnected, QWebEngineDownloadRequest::NetworkDisconnected)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkServerDown, QWebEngineDownloadRequest::NetworkServerDown)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkInvalidRequest, QWebEngineDownloadRequest::NetworkInvalidRequest)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerFailed, QWebEngineDownloadRequest::ServerFailed)
//ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerNoRange, QWebEngineDownloadRequest::ServerNoRange)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerBadContent, QWebEngineDownloadRequest::ServerBadContent)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerUnauthorized, QWebEngineDownloadRequest::ServerUnauthorized)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerCertProblem, QWebEngineDownloadRequest::ServerCertProblem)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerForbidden, QWebEngineDownloadRequest::ServerForbidden)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerUnreachable, QWebEngineDownloadRequest::ServerUnreachable)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::UserCanceled, QWebEngineDownloadRequest::UserCanceled)
//ASSERT_ENUMS_MATCH(ProfileAdapterClient::UserShutdown, QWebEngineDownloadRequest::UserShutdown)
//ASSERT_ENUMS_MATCH(ProfileAdapterClient::Crash, QWebEngineDownloadRequest::Crash)

static inline QWebEngineDownloadRequest::DownloadState toDownloadState(int state)
{
    switch (state) {
    case ProfileAdapterClient::DownloadInProgress:
        return QWebEngineDownloadRequest::DownloadInProgress;
    case ProfileAdapterClient::DownloadCompleted:
        return QWebEngineDownloadRequest::DownloadCompleted;
    case ProfileAdapterClient::DownloadCancelled:
        return QWebEngineDownloadRequest::DownloadCancelled;
    case ProfileAdapterClient::DownloadInterrupted:
        return QWebEngineDownloadRequest::DownloadInterrupted;
    default:
        Q_UNREACHABLE();
        return QWebEngineDownloadRequest::DownloadCancelled;
    }
}

static inline QWebEngineDownloadRequest::DownloadInterruptReason toDownloadInterruptReason(int reason)
{
    return static_cast<QWebEngineDownloadRequest::DownloadInterruptReason>(reason);
}

/*!
    \class QWebEngineDownloadRequest
    \brief The QWebEngineDownloadRequest class provides information about a download.

    \inmodule QtWebEngineCore

    QWebEngineDownloadRequest models a download throughout its life cycle, starting
    with a pending download request and finishing with a completed download. It
    can be used, for example, to get information about new downloads, to monitor
    progress, and to pause, resume, and cancel downloads.

    Downloads are usually triggered by user interaction on a web page. It is the
    QWebEngineProfile's responsibility to notify the application of new download
    requests, which it does by emitting the
    \l{QWebEngineProfile::downloadRequested}{downloadRequested} signal together
    with a newly created QWebEngineDownloadRequest. The application can then
    examine this item and decide whether to accept it or not. A signal handler
    must explicitly call accept() on the item for \QWE to actually start
    downloading and writing data to disk. If no signal handler calls accept(),
    then the download request will be automatically rejected and nothing will be
    written to disk.

    \note Some properties, such as setting the path and file name where the file
    will be saved (see \l downloadDirectory() and \l downloadFileName()), can
    only be changed before calling accept().

    \section2 Object Life Cycle

    All items are guaranteed to be valid during the emission of the
    \l{QWebEngineProfile::downloadRequested}{downloadRequested} signal. If
    accept() is \e not called by any signal handler, then the item will be
    deleted \e immediately after signal emission. This means that the
    application \b{must not} keep references to rejected download items. It also
    means the application should not use a queued connection to this signal.

    If accept() \e is called by a signal handler, then the QWebEngineProfile
    will take ownership of the item. However, it is safe for the application to
    delete the item at any time, except during the handling of the
    \l{QWebEngineProfile::downloadRequested}{downloadRequested} signal. The
    QWebEngineProfile being a long-lived object, it is in fact recommended that
    the application delete any items it is no longer interested in.

    \note Deleting an item will also automatically cancel a download since 5.12.2,
    but it is recommended to cancel manually before deleting for portability.

    \section2 Web Page Downloads

    In addition to normal file downloads, which consist simply of retrieving
    some raw bytes from the network and writing them to disk, \QWE also
    supports saving complete web pages, which involves parsing the page's HTML,
    downloading any dependent resources, and potentially packaging everything
    into a special file format (\l savePageFormat). To check if a download is
    for a file or a web page, use \l isSavePageDownload.

    \sa QWebEngineProfile, QWebEngineProfile::downloadRequested,
    QWebEnginePage::download, QWebEnginePage::save
*/

QWebEngineDownloadRequestPrivate::QWebEngineDownloadRequestPrivate(
        QtWebEngineCore::ProfileAdapter *adapter)
    : profileAdapter(adapter)
{
}

QWebEngineDownloadRequestPrivate::~QWebEngineDownloadRequestPrivate()
{
}

void QWebEngineDownloadRequestPrivate::update(const ProfileAdapterClient::DownloadItemInfo &info)
{
    Q_Q(QWebEngineDownloadRequest);

    Q_ASSERT(downloadState != QWebEngineDownloadRequest::DownloadRequested);

    if (toDownloadInterruptReason(info.downloadInterruptReason) != interruptReason) {
        interruptReason = toDownloadInterruptReason(info.downloadInterruptReason);
        Q_EMIT q->interruptReasonChanged();
    }
    if (toDownloadState(info.state) != downloadState) {
        downloadState = toDownloadState(info.state);
        Q_EMIT q->stateChanged(downloadState);
    }

    if (info.receivedBytes != receivedBytes || info.totalBytes != totalBytes) {

      if (info.receivedBytes != receivedBytes) {
          receivedBytes = info.receivedBytes;
          Q_EMIT q->receivedBytesChanged();
      }
      if (info.totalBytes != totalBytes) {
          totalBytes = info.totalBytes;
          Q_EMIT q->totalBytesChanged();
      }
    }

    if (info.done)
        setFinished();

    if (downloadPaused != info.paused) {
        downloadPaused = info.paused;
        Q_EMIT q->isPausedChanged();
    }
}

void QWebEngineDownloadRequestPrivate::setFinished()
{
    if (downloadFinished)
        return;

    downloadFinished = true;
    Q_EMIT q_ptr->isFinishedChanged();
}

/*!
    Accepts the current download request, which will start the download.

    If the item is in the \l DownloadRequested state, then it will transition
    into the \l DownloadInProgress state and the downloading will begin. If the
    item is in any other state, then nothing will happen.

    \sa isFinished, stateChanged()
*/

void QWebEngineDownloadRequest::accept()
{
    Q_D(QWebEngineDownloadRequest);

    if (d->downloadState != QWebEngineDownloadRequest::DownloadRequested)
        return;

    d->downloadState = QWebEngineDownloadRequest::DownloadInProgress;
    Q_EMIT stateChanged(d->downloadState);
}

/*!
    Cancels the current download.

    If the item is in the \l DownloadInProgress state, then it will transition
    into the \l DownloadCancelled state, the downloading will stop, and partially
    downloaded files will be deleted from disk.

    If the item is in the \l DownloadCompleted state, then nothing will happen.
    If the item is in any other state, then it will transition into the \l
    DownloadCancelled state without further effect.

    \sa isFinished, stateChanged()
*/

void QWebEngineDownloadRequest::cancel()
{
    Q_D(QWebEngineDownloadRequest);

    QWebEngineDownloadRequest::DownloadState state = d->downloadState;

    if (state == QWebEngineDownloadRequest::DownloadCompleted)
        return;

    bool cancelled = state == QWebEngineDownloadRequest::DownloadCancelled;
    if (cancelled)
        return;

    // Check if the download manager has a DownloadItem for this ID
    // (network downloads or in progress page/resource saves)
    if (state == QWebEngineDownloadRequest::DownloadInProgress) {
        if (d->profileAdapter)
            cancelled = d->profileAdapter->cancelDownload(d->downloadId);
    }

    // Not cancelled downloads are not even started yet at this point
    if (!cancelled) {
        d->downloadState = QWebEngineDownloadRequest::DownloadCancelled;
        Q_EMIT stateChanged(d->downloadState);
        d->setFinished();
    }
}

/*!
    Pauses the download.

    Has no effect if the state is not \l DownloadInProgress. Does not change the
    state.

    \sa resume(), isPaused()
*/

void QWebEngineDownloadRequest::pause()
{
    Q_D(QWebEngineDownloadRequest);

    QWebEngineDownloadRequest::DownloadState state = d->downloadState;

    if (state != QWebEngineDownloadRequest::DownloadInProgress)
        return;

    if (d->profileAdapter)
        d->profileAdapter->pauseDownload(d->downloadId);
}

/*!
    Resumes the current download if it was paused or interrupted.

    Has no effect if the state is not \l DownloadInProgress or \l
    DownloadInterrupted. Does not change the state.

    \sa pause(), isPaused(), state()
*/
void QWebEngineDownloadRequest::resume()
{
    Q_D(QWebEngineDownloadRequest);

    QWebEngineDownloadRequest::DownloadState state = d->downloadState;

    if (d->downloadFinished || (state != QWebEngineDownloadRequest::DownloadInProgress && state != QWebEngineDownloadRequest::DownloadInterrupted))
        return;
    if (d->profileAdapter)
        d->profileAdapter->resumeDownload(d->downloadId);
}

/*!
    Returns the download item's ID.
*/

quint32 QWebEngineDownloadRequest::id() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->downloadId;
}

/*!
    \fn void QWebEngineDownloadRequest::isPausedChanged()

    This signal is emitted whenever isPaused changes.

    \sa pause(), isPaused
*/

/*!
    \fn void QWebEngineDownloadRequest::stateChanged(DownloadState state)

    This signal is emitted whenever the download's \a state changes.

    \sa state(), DownloadState
*/

/*!
    \enum QWebEngineDownloadRequest::DownloadState

    This enum describes the state of the download:

    \value DownloadRequested Download has been requested, but has not been accepted yet.
    \value DownloadInProgress Download is in progress.
    \value DownloadCompleted Download completed successfully.
    \value DownloadCancelled Download has been cancelled.
    \value DownloadInterrupted Download has been interrupted (by the server or because of lost
            connectivity).
*/

/*!
    \enum QWebEngineDownloadRequest::SavePageFormat

    This enum describes the format that is used to save a web page.

    \value UnknownSaveFormat This is not a request for downloading a complete web page.
    \value SingleHtmlSaveFormat The page is saved as a single HTML page. Resources such as images
           are not saved.
    \value CompleteHtmlSaveFormat The page is saved as a complete HTML page, for example a directory
            containing the single HTML page and the resources.
    \value MimeHtmlSaveFormat The page is saved as a complete web page in the MIME HTML format.
*/

/*!
    \enum QWebEngineDownloadRequest::DownloadInterruptReason

    Describes the reason why a download was interrupted:

    \value NoReason Unknown reason or not interrupted.
    \value FileFailed General file operation failure.
    \value FileAccessDenied The file cannot be written locally, due to access restrictions.
    \value FileNoSpace Insufficient space on the target drive.
    \value FileNameTooLong The directory or file name is too long.
    \value FileTooLarge The file size exceeds the file system limitation.
    \value FileVirusInfected The file is infected with a virus.
    \value FileTransientError Temporary problem (for example the file is in use,
           out of memory, or too many files are opened at once).
    \value FileBlocked The file was blocked due to local policy.
    \value FileSecurityCheckFailed An attempt to check the safety of the download
           failed due to unexpected reasons.
    \value FileTooShort An attempt was made to seek past the end of a file when
           opening a file (as part of resuming a previously interrupted download).
    \value FileHashMismatch The partial file did not match the expected hash.

    \value NetworkFailed General network failure.
    \value NetworkTimeout The network operation has timed out.
    \value NetworkDisconnected The network connection has been terminated.
    \value NetworkServerDown The server has gone down.
    \value NetworkInvalidRequest The network request was invalid (for example, the
           original or redirected URL is invalid, has an unsupported scheme, or is disallowed by policy).

    \value ServerFailed General server failure.
    \value ServerBadContent The server does not have the requested data.
    \value ServerUnauthorized The server did not authorize access to the resource.
    \value ServerCertProblem A problem with the server certificate occurred.
    \value ServerForbidden Access forbidden by the server.
    \value ServerUnreachable Unexpected server response (might indicate that
           the responding server may not be the intended server).
    \value UserCanceled The user canceled the download.
*/

/*!
    Returns the download item's current state.

    \sa DownloadState
*/

QWebEngineDownloadRequest::DownloadState QWebEngineDownloadRequest::state() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->downloadState;
}

/*!
    Returns the total amount of data to download in bytes.

    \c -1 means the size is unknown.
*/

qint64 QWebEngineDownloadRequest::totalBytes() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->totalBytes;
}

/*!
    Returns the amount of data in bytes that has been downloaded so far.

    \c -1 means the size is unknown.
*/

qint64 QWebEngineDownloadRequest::receivedBytes() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->receivedBytes;
}

/*!
    Returns the download's origin URL.
*/

QUrl QWebEngineDownloadRequest::url() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->downloadUrl;
}

/*!
    Returns the MIME type of the download.
*/

QString QWebEngineDownloadRequest::mimeType() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->mimeType;
}

/*!
    Returns the download directory path.
*/

QString QWebEngineDownloadRequest::downloadDirectory() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->downloadDirectory;
}

/*!
    Sets \a directory as the directory path to download the file to.

    The download directory path can only be set in response to the QWebEngineProfile::downloadRequested()
    signal before the download is accepted. Past that point, this function has no effect on the
    download item's state.
*/

void QWebEngineDownloadRequest::setDownloadDirectory(const QString &directory)
{
    Q_D(QWebEngineDownloadRequest);
    if (d->downloadState != QWebEngineDownloadRequest::DownloadRequested) {
        qWarning("Setting the download directory is not allowed after the download has been accepted.");
        return;
    }

    if (!directory.isEmpty() && d->downloadDirectory != directory) {
        d->downloadDirectory = directory;
        Q_EMIT downloadDirectoryChanged();
    }

    if (!d->isCustomFileName && d->profileAdapter) {
        QString newFileName = QFileInfo(d->profileAdapter->determineDownloadPath(d->downloadDirectory,
                                                                                 d->suggestedFileName,
                                                                                 d->startTime)).fileName();
        if (d->downloadFileName != newFileName) {
            d->downloadFileName = newFileName;
            Q_EMIT downloadFileNameChanged();
        }
    }
}

/*!
    Returns the file name to download the file to.
*/

QString QWebEngineDownloadRequest::downloadFileName() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->downloadFileName;
}

/*!
    Sets \a fileName as the file name to download the file to.

    The download file name can only be set in response to the QWebEngineProfile::downloadRequested()
    signal before the download is accepted. Past that point, this function has no effect on the
    download item's state.
*/

void QWebEngineDownloadRequest::setDownloadFileName(const QString &fileName)
{
    Q_D(QWebEngineDownloadRequest);
    if (d->downloadState != QWebEngineDownloadRequest::DownloadRequested) {
        qWarning("Setting the download file name is not allowed after the download has been accepted.");
        return;
    }

    if (!fileName.isEmpty()) {
        d->downloadFileName = fileName;
        d->isCustomFileName = true;
        Q_EMIT downloadFileNameChanged();
    }
}

/*!
    Returns the suggested file name.
*/

QString QWebEngineDownloadRequest::suggestedFileName() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->suggestedFileName;
}

/*!
    \property QWebEngineDownloadRequest::isFinished
    \brief Whether this download is finished (completed, cancelled,
           or non-resumable interrupted state).

    \sa state()
*/

bool QWebEngineDownloadRequest::isFinished() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->downloadFinished;
}

/*!
    \property QWebEngineDownloadRequest::isPaused
    \brief Whether this download is paused.

    \sa pause(), resume()
*/

bool QWebEngineDownloadRequest::isPaused() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->downloadPaused;
}

/*!
    Returns the format the web page will be saved in if this is a download request for a web page.
    \sa setSavePageFormat(), isSavePageDownload()
*/
QWebEngineDownloadRequest::SavePageFormat QWebEngineDownloadRequest::savePageFormat() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->savePageFormat;
}

/*!
    Sets the \a format the web page will be saved in if this is a download request for a web page.

    \sa savePageFormat(), isSavePageDownload()
*/
void QWebEngineDownloadRequest::setSavePageFormat(QWebEngineDownloadRequest::SavePageFormat format)
{
  Q_D(QWebEngineDownloadRequest);
  if (d->savePageFormat != format) {
      d->savePageFormat = format;
      Q_EMIT savePageFormatChanged();
  }
}

/*!
    Returns \c true if this is a download request for saving a web page.

    \sa savePageFormat(), setSavePageFormat()
 */
bool QWebEngineDownloadRequest::isSavePageDownload() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->isSavePageDownload;
}

/*!
    Returns the reason why the download was interrupted.

    \sa interruptReasonString()
*/

QWebEngineDownloadRequest::DownloadInterruptReason QWebEngineDownloadRequest::interruptReason() const
{
    Q_D(const QWebEngineDownloadRequest);
    return d->interruptReason;
}

/*!
    Returns a human-readable description of the reason for interrupting the download.

    \sa interruptReason()
*/

QString QWebEngineDownloadRequest::interruptReasonString() const
{
    return ProfileAdapterClient::downloadInterruptReasonToString(
              static_cast<ProfileAdapterClient::DownloadInterruptReason>(interruptReason()));
}

/*!
    Returns the page the download was requested on. If the download was not triggered by content in a page,
    \c nullptr is returned.
*/
QWebEnginePage *QWebEngineDownloadRequest::page() const
{
    Q_D(const QWebEngineDownloadRequest);
    if (d->adapterClient->clientType() == QtWebEngineCore::WebContentsAdapterClient::WidgetsClient)
        return const_cast<QWebEnginePage *>(static_cast<const QWebEnginePage *>(d->adapterClient->holdingQObject()));
    return nullptr;
}


/*! \internal
*/
QWebEngineDownloadRequest::QWebEngineDownloadRequest(QWebEngineDownloadRequestPrivate *p, QObject *parent)
    : QObject(parent)
    , d_ptr(p)
{
    p->q_ptr = this;
}

/*! \internal
*/
QWebEngineDownloadRequest::~QWebEngineDownloadRequest()
{
    // MEMO Items are owned by profile by default and will be destroyed on profile's destruction
    //      It's not safe to access profile in that case, so we rely on profile to clean up items
    if (!isFinished())
        cancel();
}

QT_END_NAMESPACE

#include "moc_qwebenginedownloadrequest.cpp"
