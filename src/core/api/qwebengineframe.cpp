// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebengineframe.h"

#include "qwebenginescript.h"
#include <QtQml/qqmlengine.h>
#include <QtGui/qpagelayout.h>
#include <QtGui/qpageranges.h>

#include "web_contents_adapter_client.h"
#include "web_contents_adapter.h"

QT_BEGIN_NAMESPACE

#define LOCK_ADAPTER(adapter_variable, return_value)                                               \
    auto adapter = m_adapter.lock();                                                               \
    if (!adapter)                                                                                  \
    return return_value

/*!
    \class QWebEngineFrame
    \brief The QWebEngineFrame class gives information about and control over a page frame.
    \since 6.8

    \inmodule QtWebEngineCore

    A web engine frame represents a single frame within a web page, such as those created by
    \c <frame> or \c <iframe> HTML elements.
    An active QWebEnginePage has one or more frames arranged in a tree structure. The top-level
    frame, the root of this tree, can be accessed through the mainFrame() method, and
    children() provides a frame's direct descendants.

    A frame's lifetime is, at most, as long as the QWebEnginePage object that produced it.
    However, frames may be created and deleted spontaneously and dynamically, for example through
    navigation and script execution. Because of this, many QWebEngineFrame methods return
    optional values, which will be \c std::nullopt if the frame no longer exists.
*/

/*! \internal
 */
QWebEngineFrame::QWebEngineFrame(QWeakPointer<QtWebEngineCore::WebContentsAdapter> adapter,
                                 quint64 id)
    : m_adapter(std::move(adapter)), m_id(id)
{
}

/*!
    Returns \c{true} if this object represents an existing frame; \c{false} otherwise.

    Once a frame is invalid, it never becomes valid again.
*/
bool QWebEngineFrame::isValid() const
{
    LOCK_ADAPTER(adapter, false);
    return adapter->hasFrame(m_id);
}

/*!
    Returns the frame name; that is, what would be returned by \c window.name in JavaScript.

    If the frame could not be found, returns a null QString.

    \sa htmlName
*/
QString QWebEngineFrame::name() const
{
    LOCK_ADAPTER(adapter, QString());
    return adapter->frameName(m_id);
}

/*!
    Returns the value of the frame's \c name HTML attribute, or an empty string if it has none.

    If the frame could not be found, returns a null QString.

    \sa name
*/
QString QWebEngineFrame::htmlName() const
{
    LOCK_ADAPTER(adapter, QString());
    return adapter->frameHtmlName(m_id);
}

/*!
    Returns a list of the frame's children in an arbitrary order.

    If the frame could not be found, returns an empty list.
 */
QList<QWebEngineFrame> QWebEngineFrame::children() const
{
    LOCK_ADAPTER(adapter, {});
    QList<QWebEngineFrame> result;
    for (auto childId : adapter->frameChildren(m_id))
        result.push_back(QWebEngineFrame{ m_adapter, childId });
    return result;
}

/*!
    Returns the URL of the content currently loaded in this frame.

    If the frame could not be found, returns an empty QUrl.
 */
QUrl QWebEngineFrame::url() const
{
    LOCK_ADAPTER(adapter, QUrl());
    return adapter->frameUrl(m_id);
}

/*!
    Returns the size of the frame within the viewport.

    If the frame could not be found, returns QSizeF().
 */
QSizeF QWebEngineFrame::size() const
{
    LOCK_ADAPTER(adapter, QSizeF());
    return adapter->frameSize(m_id);
}

/*!
    Returns \c{true} if this object represents the page's main frame; \c{false} otherwise.
*/
bool QWebEngineFrame::isMainFrame() const
{
    LOCK_ADAPTER(adapter, false);
    return adapter->mainFrameId() == m_id;
}

/*! \fn void QWebEngineFrame::runJavaScript(const QString &script, const std::function<void(const QVariant &)> &callback)
    \fn void QWebEngineFrame::runJavaScript(const QString &script, quint32 worldId)
    \fn void QWebEngineFrame::runJavaScript(const QString &script, quint32 worldId, const
   std::function<void(const QVariant &)> &callback)

    Runs the JavaScript code contained in \a script on this frame, without checking
    whether the DOM of the page has been constructed.
    To avoid conflicts with other scripts executed on the page, the world in
    which the script is run is specified by \a worldId. The world ID values are
    the same as provided by QWebEngineScript::ScriptWorldId, and between \c 0
    and \c 256. If you leave out the \c world ID, the script is run in the
    \c MainWorld.
    When the script has been executed, \a callback is called with the result of the last
    executed statement. \c callback can be any of a function pointer, a functor or a lambda,
    and it is expected to take a QVariant parameter. For example:
    \code
    page.runJavaScript("document.title", [](const QVariant &v) { qDebug() << v.toString(); });
    \endcode
    Only plain data can be returned from JavaScript as the result value.
    Supported data types include all of the JSON data types as well as, for
    example, \c{Date} and \c{ArrayBuffer}. Unsupported data types include, for
    example, \c{Function} and \c{Promise}.
    \warning Do not execute lengthy routines in the callback function, because it might block the
    rendering of the web engine page.
    \warning We guarantee that the \a callback is always called, but it might be
   done during page destruction. When QWebEnginePage is deleted, the callback is triggered with an
   invalid value and it is not safe to use the corresponding QWebEnginePage or QWebEngineView
   instance inside it.
    \sa QWebEngineScript::ScriptWorldId, QWebEnginePage::runJavaScript, {Script Injection}
 */
void QWebEngineFrame::runJavaScript(const QString &script,
                                    const std::function<void(const QVariant &)> &callback)
{
    runJavaScript(script, QWebEngineScript::MainWorld, callback);
}

void QWebEngineFrame::runJavaScript(const QString &script, quint32 worldId,
                                    const std::function<void(const QVariant &)> &callback)
{
    LOCK_ADAPTER(adapter, );
    adapter->runJavaScript(script, worldId, m_id, callback);
}

void QWebEngineFrame::runJavaScript(const QString &script, quint32 worldId)
{
    runJavaScript(script, worldId, std::function<void(const QVariant &)>{});
}

void QWebEngineFrame::runJavaScript(const QString &script, const QJSValue &callback)
{
    runJavaScript(script, QWebEngineScript::MainWorld, callback);
}

void QWebEngineFrame::runJavaScript(const QString &script, quint32 worldId,
                                    const QJSValue &callback)
{
    LOCK_ADAPTER(adapter, );
    std::function<void(const QVariant &)> wrappedCallback;
    if (!callback.isUndefined()) {
        const QObject *holdingObject = adapter->adapterClient()->holdingQObject();
        wrappedCallback = [holdingObject, callback](const QVariant &result) {
            if (auto engine = qmlEngine(holdingObject)) {
                QJSValueList args;
                args.append(engine->toScriptValue(result));
                callback.call(args);
            } else {
                qWarning("No QML engine found to execute runJavaScript() callback");
            }
        };
    }
    runJavaScript(script, worldId, wrappedCallback);
}

/*!
    Renders the current content of the frame into a PDF document and saves it in the location
    specified in \a filePath. Printing uses a page size of A4, portrait layout, and includes the
    full range of pages.

    This method issues an asynchronous request for printing the web page into a PDF and returns
    immediately. To be informed about the result of the request, connect to the \l
    QWebEnginePage::pdfPrintingFinished() signal.

    \note The \l QWebEnginePage::Stop web action can be used to interrupt this asynchronous
    operation.

    If a file already exists at the provided file path, it will be overwritten.

    \sa QWebEnginePage::pdfPrintingFinished()
 */
void QWebEngineFrame::printToPdf(const QString &filePath)
{
    LOCK_ADAPTER(adapter, );
    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF());
    adapter->adapterClient()->printToPdf(filePath, layout, QPageRanges(), m_id);
}

/*!
    Renders the current content of the frame into a PDF document and returns a byte array containing
    the PDF data as parameter to \a callback. Printing uses a page size of A4, portrait layout, and
    includes the full range of pages.

    The \a callback must take a const reference to a QByteArray as parameter. If printing was
    successful, this byte array will contain the PDF data, otherwise, the byte array will be empty.

    \note The \l QWebEnginePage::Stop web action can be used to interrupt this operation.
*/
void QWebEngineFrame::printToPdf(const std::function<void(const QByteArray &)> &callback)
{
    LOCK_ADAPTER(adapter, );
    std::function wrappedCallback = [callback](QSharedPointer<QByteArray> result) {
        if (callback)
            callback(result ? *result : QByteArray());
    };
    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF());
    adapter->adapterClient()->printToPdf(std::move(wrappedCallback), layout, QPageRanges(), m_id);
}

void QWebEngineFrame::printToPdf(const QJSValue &callback)
{
    LOCK_ADAPTER(adapter, );
    std::function<void(QSharedPointer<QByteArray>)> wrappedCallback;
    if (!callback.isUndefined()) {
        const QObject *holdingObject = adapter->adapterClient()->holdingQObject();
        wrappedCallback = [holdingObject, callback](QSharedPointer<QByteArray> result) {
            if (auto engine = qmlEngine(holdingObject)) {
                QJSValueList args;
                args.append(engine->toScriptValue(result ? *result : QByteArray()));
                callback.call(args);
            } else {
                qWarning("No QML engine found to execute runJavaScript() callback");
            }
        };
    }
    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF());
    adapter->adapterClient()->printToPdf(std::move(wrappedCallback), layout, QPageRanges(), m_id);
}

/*! \fn bool QWebEngineFrame::operator==(const QWebEngineFrame &left, const QWebEngineFrame &right) noexcept

    Returns \c{true} if \a left and \a right represent the same frame in the same web page,
   otherwise \c{false}.
 */

/*! \fn bool QWebEngineFrame::operator!=(const QWebEngineFrame &left, const QWebEngineFrame &right) noexcept

    Returns \c{true} if \a left and \a right represent different frames in the same web page,
   otherwise \c{false}.
 */

QT_END_NAMESPACE

#include "moc_qwebengineframe.cpp"
