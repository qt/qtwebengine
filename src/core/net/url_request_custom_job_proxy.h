// Copyright (C) 2017 Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef URL_REQUEST_CUSTOM_JOB_PROXY_H_
#define URL_REQUEST_CUSTOM_JOB_PROXY_H_

#include "base/memory/weak_ptr.h"
#include "base/task/sequenced_task_runner.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "url/gurl.h"
#include "url/origin.h"
#include <QtCore/QPointer>
#include <QMap>
#include <QByteArray>

QT_FORWARD_DECLARE_CLASS(QIODevice)

namespace QtWebEngineCore {

class URLRequestCustomJob;
class URLRequestCustomJobDelegate;
class ProfileAdapter;

// Used to comunicate between URLRequestCustomJob living on the IO thread
// and URLRequestCustomJobDelegate living on the UI thread.
class URLRequestCustomJobProxy : public base::RefCountedThreadSafe<URLRequestCustomJobProxy>
{

public:
    class Client {
    public:
        std::string m_mimeType;
        std::string m_charset;
        QMultiMap<QByteArray, QByteArray> m_additionalResponseHeaders;
        GURL m_redirect;
        QIODevice *m_device;
        int64_t m_firstBytePosition;
        int m_error;
        virtual void notifyExpectedContentSize(qint64 size) = 0;
        virtual void notifyHeadersComplete() = 0;
        virtual void notifyCanceled() = 0;
        virtual void notifyAborted() = 0;
        virtual void notifyStartFailure(int) = 0;
        virtual void notifyReadyRead() = 0;
        virtual base::SequencedTaskRunner *taskRunner() = 0;
    };

    URLRequestCustomJobProxy(Client *client,
                             const std::string &scheme,
                             QPointer<ProfileAdapter> profileAdapter);
    ~URLRequestCustomJobProxy();

    // Called from URLRequestCustomJobDelegate via post:
    //void setReplyCharset(const std::string &);
    void reply(std::string mimeType, QIODevice *device,
               QMultiMap<QByteArray, QByteArray> additionalResponseHeaders);
    void redirect(GURL url);
    void abort();
    void fail(int error);
    void release();
    void initialize(GURL url, std::string method, absl::optional<url::Origin> initiatorOrigin, std::map<std::string, std::string> headers);
    void readyRead();

    // IO thread owned:
    Client *m_client;
    bool m_started;

    // UI thread owned:
    std::string m_scheme;
    URLRequestCustomJobDelegate *m_delegate;
    QPointer<ProfileAdapter> m_profileAdapter;
    scoped_refptr<base::SequencedTaskRunner> m_ioTaskRunner;
};

} // namespace QtWebEngineCore

#endif // URL_REQUEST_CUSTOM_JOB_PROXY_H_
