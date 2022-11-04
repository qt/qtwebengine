// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "webview.h"
#include <QWebEngineSettings>

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
{
    const QString html = QStringLiteral(
            "<html><head>"
            "   <style>"
            "       input {"
            "           width: 250px; height: 50px; font-size: 28pt;"
            "           position: absolute; top: 50%; left: 50%; margin-left: -125px; margin-top: -25px;"
            "           font-family: serif"
            "       }"
            "       body { background-color: black; }"
            "   </style>"
            "</head>"
            "<body onload='document.getElementById(\"input1\").focus();'>"
            "   <input type='text' id='input1' />"
            "</body></html>");

    settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    setHtml(html);
}
