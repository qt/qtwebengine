/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TESTHANDLER_H
#define TESTHANDLER_H

#include <QWebEngineCertificateError>

class TestHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QWebEngineCertificateError certificateError READ certificateError WRITE
                       setCertificateError NOTIFY certificateErrorChanged)
    Q_PROPERTY(bool loadSuccess READ loadSuccess WRITE setLoadSuccess NOTIFY loadSuccessChanged)
public:
    explicit TestHandler(QObject *parent = nullptr);
    QWebEngineCertificateError certificateError() const;

    void setCertificateError(QWebEngineCertificateError error);
    void setLoadSuccess(bool success);
    bool loadSuccess() const;
    void load(const QUrl &page);

signals:
    void loadPage(const QUrl &page);
    void certificateErrorChanged();
    void loadSuccessChanged();

private:
    QWebEngineCertificateError *m_error = nullptr;
    bool m_loadSuccess = false;
};

#endif // TESTHANDLER_H
