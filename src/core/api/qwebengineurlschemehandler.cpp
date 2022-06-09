// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineurlschemehandler.h"

#include "qwebengineurlrequestjob.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineUrlSchemeHandler
    \brief The QWebEngineUrlSchemeHandler class is a base class for handling custom URL schemes.
    \since 5.6

    To implement a custom URL scheme for QtWebEngine, you first have to create an instance of
    QWebEngineUrlScheme and register it using QWebEngineUrlScheme::registerScheme().

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
        void requestStarted(QWebEngineUrlRequestJob *request)
        {
            // ....
        }
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

    \sa {QWebEngineUrlScheme}, {WebEngine Widgets WebUI Example}
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
