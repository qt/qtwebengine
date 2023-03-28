// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WEBPOPUPWINDOW_H
#define WEBPOPUPWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QWebEngineProfile;
class QWebEngineView;
QT_END_NAMESPACE

class WebView;

class WebPopupWindow : public QWidget
{
    Q_OBJECT

public:
    explicit WebPopupWindow(QWebEngineProfile *profile);
    WebView *view() const;

private slots:
    void handleGeometryChangeRequested(const QRect &newGeometry);

private:
    QLineEdit *m_urlLineEdit;
    QAction *m_favAction;
    WebView *m_view;
};
#endif // WEBPOPUPWINDOW_H
