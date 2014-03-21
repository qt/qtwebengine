/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef WEB_CONTENTS_ADAPTER_H
#define WEB_CONTENTS_ADAPTER_H

#include "qtwebenginecoreglobal.h"
#include "web_contents_adapter_client.h"

#include <QScopedPointer>
#include <QSharedData>
#include <QString>
#include <QUrl>

namespace content {
class WebContents;
}
class WebContentsAdapterPrivate;

class QWEBENGINE_EXPORT WebContentsAdapter : public QSharedData {
public:
    static QExplicitlySharedDataPointer<WebContentsAdapter> createFromSerializedNavigationHistory(QDataStream &input, WebContentsAdapterClient *adapterClient, WebContentsAdapterClient::RenderingMode renderingMode);
    // Takes ownership of the WebContents.
    WebContentsAdapter(WebContentsAdapterClient::RenderingMode renderingMode, content::WebContents *webContents = 0);
    ~WebContentsAdapter();
    void initialize(WebContentsAdapterClient *adapterClient);

    bool canGoBack() const;
    bool canGoForward() const;
    bool isLoading() const;
    void stop();
    void reload();
    void load(const QUrl&);
    void setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl, const QUrl &unreachableUrl);
    QUrl activeUrl() const;
    QUrl requestedUrl() const;
    QString pageTitle() const;
    QString selectedText() const;

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void pasteAndMatchStyle();
    void selectAll();

    void navigateToIndex(int);
    void navigateToOffset(int);
    int navigationEntryCount();
    int currentNavigationEntryIndex();
    QUrl getNavigationEntryOriginalUrl(int index);
    QUrl getNavigationEntryUrl(int index);
    QString getNavigationEntryTitle(int index);
    void clearNavigationHistory();
    void serializeNavigationHistory(QDataStream &output);
    void setZoomFactor(qreal);
    qreal currentZoomFactor() const;
    void enableInspector(bool);
    void filesSelectedInChooser(const QStringList &fileList, WebContentsAdapterClient::FileChooserMode);
    void runJavaScript(const QString &javaScript, const QString &xPath);
    quint64 runJavaScriptCallbackResult(const QString &javaScript, const QString &xPath);
    quint64 fetchDocumentMarkup();
    quint64 fetchDocumentInnerText();
    quint64 findText(const QString &subString, bool caseSensitively, bool findBackward);
    void stopFinding();

    void wasShown();
    void wasHidden();

    void dpiScaleChanged();

private:
    Q_DISABLE_COPY(WebContentsAdapter);
    Q_DECLARE_PRIVATE(WebContentsAdapter);
    QScopedPointer<WebContentsAdapterPrivate> d_ptr;
};
#endif // WEB_CONTENTS_ADAPTER_H
