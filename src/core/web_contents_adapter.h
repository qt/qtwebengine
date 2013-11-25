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

QT_FORWARD_DECLARE_CLASS(QVariant)
namespace content {
class WebContents;
}
class WebContentsAdapterPrivate;

struct JSCallbackBase {
    virtual ~JSCallbackBase() {}
    virtual void call(const QVariant&) = 0;
};

class QWEBENGINE_EXPORT WebContentsAdapter : public QSharedData {
public:
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
    QUrl activeUrl() const;
    QString pageTitle() const;

    void navigateToIndex(int);
    void navigateToOffset(int);
    int navigationEntryCount();
    int currentNavigationEntryIndex();
    QUrl getNavigationEntryOriginalUrl(int index);
    QUrl getNavigationEntryUrl(int index);
    QString getNavigationEntryTitle(int index);
    void clearNavigationHistory();
    void setZoomFactor(qreal);
    qreal currentZoomFactor() const;
    void enableInspector(bool);
    void runJavaScript(const QString &javaScript, const QString &xPath = QString(), JSCallbackBase * = 0);
    void filesSelectedInChooser(const QStringList &fileList, WebContentsAdapterClient::FileChooserMode);

    void dpiScaleChanged();

private:
    Q_DISABLE_COPY(WebContentsAdapter);
    Q_DECLARE_PRIVATE(WebContentsAdapter);
    QScopedPointer<WebContentsAdapterPrivate> d_ptr;
};
#endif // WEB_CONTENTS_ADAPTER_H
