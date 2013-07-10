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

#ifndef QWEBCONTESTSVIEW_H
#define QWEBCONTESTSVIEW_H

#include <qtwebengineglobal.h>

#include <QWidget>
#include <QScopedPointer>

class QWebContentsViewPrivate;

class QWEBENGINEWIDGETS_EXPORT QWebContentsView : public QWidget {
    Q_OBJECT
public:
    QWebContentsView();
    ~QWebContentsView();

    void load(const QUrl& url);
    bool canGoBack() const;
    bool canGoForward() const;

public Q_SLOTS:
    void back();
    void forward();
    void reload();
    void stop();

Q_SIGNALS:
    void loadFinished(bool ok);
    void loadStarted();
    void titleChanged(const QString& title);
    void urlChanged(const QUrl& url);

private:
    Q_DECLARE_PRIVATE(QWebContentsView);
    QWebContentsViewPrivate *d_ptr;
};

#endif // QWEBCONTESTSVIEW_H
