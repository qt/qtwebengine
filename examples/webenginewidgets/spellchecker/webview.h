// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebEngineView>

class WebView : public QWebEngineView
{
    Q_OBJECT

public:
    WebView(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QMap<QString,QString> m_spellCheckLanguages;
};

#endif
