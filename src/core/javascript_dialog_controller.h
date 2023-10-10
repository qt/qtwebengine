// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef JAVASCRIPT_DIALOG_CONTROLLER_H
#define JAVASCRIPT_DIALOG_CONTROLLER_H

#include "web_contents_adapter_client.h"

QT_FORWARD_DECLARE_CLASS(QString)

namespace QtWebEngineCore {

class JavaScriptDialogControllerPrivate;

class Q_WEBENGINECORE_PRIVATE_EXPORT JavaScriptDialogController : public QObject {
    Q_OBJECT
public:
    ~JavaScriptDialogController();
    QString message() const;
    QString defaultPrompt() const;
    QString title() const;
    WebContentsAdapterClient::JavascriptDialogType type() const;
    QUrl securityOrigin() const;

public Q_SLOTS:
    void textProvided(const QString &text);
    void accept();
    void reject();

Q_SIGNALS:
    void dialogCloseRequested();

private:
    JavaScriptDialogController(JavaScriptDialogControllerPrivate *);

    QScopedPointer<JavaScriptDialogControllerPrivate> d;
    friend class JavaScriptDialogManagerQt;
};

} // namespace QtWebEngineCore

#endif // JAVASCRIPT_DIALOG_CONTROLLER_H
