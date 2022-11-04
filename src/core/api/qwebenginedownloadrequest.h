// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEDOWNLOADREQUEST_H
#define QWEBENGINEDOWNLOADREQUEST_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QWebEngineDownloadRequestPrivate;
class QWebEnginePage;
class QWebEngineProfilePrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineDownloadRequest : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(quint32 id READ id CONSTANT FINAL)
    Q_PROPERTY(DownloadState state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(SavePageFormat savePageFormat READ savePageFormat WRITE setSavePageFormat NOTIFY savePageFormatChanged FINAL)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY totalBytesChanged FINAL)
    Q_PROPERTY(qint64 receivedBytes READ receivedBytes NOTIFY receivedBytesChanged FINAL)
    Q_PROPERTY(QString mimeType READ mimeType FINAL)
    Q_PROPERTY(DownloadInterruptReason interruptReason READ interruptReason NOTIFY interruptReasonChanged FINAL)
    Q_PROPERTY(QString interruptReasonString READ interruptReasonString NOTIFY interruptReasonChanged FINAL)
    Q_PROPERTY(bool isFinished READ isFinished NOTIFY isFinishedChanged FINAL)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY isPausedChanged FINAL)
    Q_PROPERTY(bool isSavePageDownload READ isSavePageDownload CONSTANT FINAL)
    Q_PROPERTY(QUrl url READ url CONSTANT FINAL)
    Q_PROPERTY(QString suggestedFileName READ suggestedFileName CONSTANT FINAL)
    Q_PROPERTY(QString downloadDirectory READ downloadDirectory WRITE setDownloadDirectory NOTIFY downloadDirectoryChanged FINAL)
    Q_PROPERTY(QString downloadFileName READ downloadFileName WRITE setDownloadFileName NOTIFY downloadFileNameChanged FINAL)

    ~QWebEngineDownloadRequest() override;

    enum DownloadState {
        DownloadRequested,
        DownloadInProgress,
        DownloadCompleted,
        DownloadCancelled,
        DownloadInterrupted
    };
    Q_ENUM(DownloadState)

    enum SavePageFormat {
        UnknownSaveFormat = -1,
        SingleHtmlSaveFormat,
        CompleteHtmlSaveFormat,
        MimeHtmlSaveFormat
    };
    Q_ENUM(SavePageFormat)

    enum DownloadInterruptReason {
        NoReason = 0,
        FileFailed = 1,
        FileAccessDenied = 2,
        FileNoSpace = 3,
        FileNameTooLong = 5,
        FileTooLarge = 6,
        FileVirusInfected = 7,
        FileTransientError = 10,
        FileBlocked = 11,
        FileSecurityCheckFailed = 12,
        FileTooShort = 13,
        FileHashMismatch = 14,
        NetworkFailed = 20,
        NetworkTimeout = 21,
        NetworkDisconnected = 22,
        NetworkServerDown = 23,
        NetworkInvalidRequest = 24,
        ServerFailed = 30,
        //ServerNoRange = 31,
        ServerBadContent = 33,
        ServerUnauthorized = 34,
        ServerCertProblem = 35,
        ServerForbidden = 36,
        ServerUnreachable = 37,
        UserCanceled = 40,
        //UserShutdown = 41,
        //Crash = 50
    };
    Q_ENUM(DownloadInterruptReason)

    quint32 id() const;
    DownloadState state() const;
    qint64 totalBytes() const;
    qint64 receivedBytes() const;
    QUrl url() const;
    QString mimeType() const;
    bool isFinished() const;
    bool isPaused() const;
    SavePageFormat savePageFormat() const;
    void setSavePageFormat(SavePageFormat format);
    DownloadInterruptReason interruptReason() const;
    QString interruptReasonString() const;
    bool isSavePageDownload() const;
    QString suggestedFileName() const;
    QString downloadDirectory() const;
    void setDownloadDirectory(const QString &directory);
    QString downloadFileName() const;
    void setDownloadFileName(const QString &fileName);

    QWebEnginePage *page() const;

public Q_SLOTS:
    void accept();
    void cancel();
    void pause();
    void resume();

Q_SIGNALS:
    void stateChanged(QWebEngineDownloadRequest::DownloadState state);
    void savePageFormatChanged();
    void receivedBytesChanged();
    void totalBytesChanged();
    void interruptReasonChanged();
    void isFinishedChanged();
    void isPausedChanged();
    void downloadDirectoryChanged();
    void downloadFileNameChanged();

private:
    Q_DISABLE_COPY(QWebEngineDownloadRequest)
    Q_DECLARE_PRIVATE(QWebEngineDownloadRequest)

    friend class QWebEngineProfilePrivate;

protected:
    QWebEngineDownloadRequest(QWebEngineDownloadRequestPrivate *, QObject *parent = nullptr);
    QScopedPointer<QWebEngineDownloadRequestPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBENGINEDOWNLOADREQUEST_H
