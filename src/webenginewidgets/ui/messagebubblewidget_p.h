/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
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
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MESSAGEBUBBLEWIDGET_P_H
#define MESSAGEBUBBLEWIDGET_P_H

#include <QWidget>
#include <QPainterPath>

QT_BEGIN_NAMESPACE
class QLabel;
class QWebEngineView;
QT_END_NAMESPACE

namespace QtWebEngineWidgetUI {

class MessageBubbleWidget : public QWidget
{
    Q_OBJECT
public:
    MessageBubbleWidget();
    ~MessageBubbleWidget();

    static void showBubble(QWebEngineView *view, const QRect &anchor, const QString &mainText, const QString &subText = QString());
    static void hideBubble();
    static void moveBubble(QWebEngineView *view, const QRect &anchor);

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:
    void createBubble(const int maxWidth, const QString &mainText, const QString &subText);
    void moveToAnchor(QWebEngineView *view, const QRect &anchor);

    QPainterPath drawBoxPath(const QPoint &pos, int border, bool roundedCorners);

    QScopedPointer<QLabel> m_mainLabel;
    QScopedPointer<QLabel> m_subLabel;
    QPixmap m_pixmap;
};

} // namespace QtWebEngineWidgetUI

#endif // MESSAGEBUBBLEWIDGET_P_H
