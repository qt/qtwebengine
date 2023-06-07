// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineurlschemehandler.h"

#include "qwebengineurlrequestjob.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineUrlSchemeHandler
    \brief The QWebEngineUrlSchemeHandler class is a base class for handling custom URL schemes.
    \since 5.6

    A custom scheme handler is, broadly speaking, similar to a web application
    served over HTTP. However, because custom schemes are integrated directly
    into the web engine, they have the advantage in terms of efficiency and security:
    There is no need for generating and parsing HTTP messages or for transferring data
    over sockets, nor any way to intercept or monitor the traffic.

    To implement a custom URL scheme for QtWebEngine, you first have to create an instance of
    QWebEngineUrlScheme and register it using QWebEngineUrlScheme::registerScheme().

    As custom schemes are integrated directly into the web engine, they do not
    necessarily need to follow the standard security rules which apply to
    ordinary web content. Depending on the chosen configuration, content served
    over a custom scheme may be given access to local resources, be set to
    ignore Content-Security-Policy rules, or conversely, be denied access to any
    other content entirely. If it is to be accessed by normal content, ensure cross-origin
    access is enabled, and if accessed from HTTPS that it is marked as secure.

    \note Make sure that you create and register the scheme object \e before the QGuiApplication
    or QApplication object is instantiated.

    Then you must create a class derived from QWebEngineUrlSchemeHandler,
    and reimplement the requestStarted() method.

    Finally, install the scheme handler object via QWebEngineProfile::installUrlSchemeHandler()
    or QQuickWebEngineProfile::installUrlSchemeHandler().

    \code

    class MySchemeHandler : public QWebEngineUrlSchemeHandler
    {
    public:
        MySchemeHandler(QObject *parent = nullptr);
        void requestStarted(QWebEngineUrlRequestJob *job)
        {
            const QByteArray method = job->requestMethod();
            const QUrl url = job->requestUrl();

            if (isValidUrl(url)) {
                if (method == QByteArrayLiteral("GET")) {
                    job->reply(QByteArrayLiteral("text/html"), makeReply(url));
                else // Unsupported method
                    job->fail(QWebEngineUrlRequestJob::RequestDenied);
            } else {
                // Invalid URL
                job->fail(QWebEngineUrlRequestJob::UrlNotFound);
            }
        }
        bool isValidUrl(const QUrl &url) const // ....
        QIODevice *makeReply(const QUrl &url) // ....
    };

    int main(int argc, char **argv)
    {
        QWebEngineUrlScheme scheme("myscheme");
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort);
        scheme.setDefaultPort(2345);
        scheme.setFlags(QWebEngineUrlScheme::SecureScheme);
        QWebEngineUrlScheme::registerScheme(scheme);

        // ...
        QApplication app(argc, argv);
        // ...

        // installUrlSchemeHandler does not take ownership of the handler.
        MySchemeHandler *handler = new MySchemeHandler(parent);
        QWebEngineProfile::defaultProfile()->installUrlSchemeHandler("myscheme", handler);
    }
    \endcode

    \inmodule QtWebEngineCore

    \sa {QWebEngineUrlScheme}
*/

/*!
    Constructs a new URL scheme handler.

    The handler is created with the parent \a parent.

  */
QWebEngineUrlSchemeHandler::QWebEngineUrlSchemeHandler(QObject *parent)
    : QObject(parent)
{
}

/*!
    Deletes a custom URL scheme handler.
*/
QWebEngineUrlSchemeHandler::~QWebEngineUrlSchemeHandler()
{
}

/*!
    \fn void QWebEngineUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)

    This method is called whenever a request \a request for the registered scheme is started.

    This method must be reimplemented by all custom URL scheme handlers.
    The request is asynchronous and does not need to be handled right away.

    \sa QWebEngineUrlRequestJob
*/

QT_END_NAMESPACE

#include "moc_qwebengineurlschemehandler.cpp"
