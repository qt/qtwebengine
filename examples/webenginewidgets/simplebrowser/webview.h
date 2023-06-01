// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QIcon>
#include <QWebEngineView>
#include <QWebEngineCertificateError>
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
#include <QWebEngineFileSystemAccessRequest>
#endif
#include <QWebEnginePage>
#include <QWebEngineRegisterProtocolHandlerRequest>

class WebPage;

class WebView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebView(QWidget *parent = nullptr);
    void setPage(WebPage *page);

    int loadProgress() const;
    bool isWebActionEnabled(QWebEnginePage::WebAction webAction) const;
    QIcon favIcon() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;

signals:
    void webActionEnabledChanged(QWebEnginePage::WebAction webAction, bool enabled);
    void favIconChanged(const QIcon &icon);
    void devToolsRequested(QWebEnginePage *source);
private slots:
    void handleCertificateError(QWebEngineCertificateError error);
    void handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth);
    void handleFeaturePermissionRequested(const QUrl &securityOrigin,
                                          QWebEnginePage::Feature feature);
    void handleProxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth,
                                           const QString &proxyHost);
    void handleRegisterProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest request);
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    void handleFileSystemAccessRequested(QWebEngineFileSystemAccessRequest request);
#endif

private:
    void createWebActionTrigger(QWebEnginePage *page, QWebEnginePage::WebAction);

private:
    int m_loadProgress = 100;
};

#endif
