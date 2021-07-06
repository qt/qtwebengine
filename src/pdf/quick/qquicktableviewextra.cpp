/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquicktableviewextra_p.h"
#include <QtQml>
#include <QQmlContext>

Q_LOGGING_CATEGORY(qLcTVE, "qt.pdf.tableextra")

QT_BEGIN_NAMESPACE

/*!
    \internal
    \qmltype TableViewExtra
    \instantiates QQuickTableViewExtra
    \inqmlmodule QtQuick.Pdf
    \ingroup pdf
    \brief A helper class with missing TableView functions
    \since 5.15

    TableViewExtra provides equivalents for some functions that will be added
    to TableView in Qt 6.
*/

QQuickTableViewExtra::QQuickTableViewExtra(QObject *parent) : QObject(parent)
{
}

QPoint QQuickTableViewExtra::cellAtPos(qreal x, qreal y) const
{
    QPointF position(x, y);
    return m_tableView->cellAtPos(position);
}

QQuickItem *QQuickTableViewExtra::itemAtCell(const QPoint &cell) const
{
    return m_tableView->itemAtCell(cell);
}

void QQuickTableViewExtra::positionViewAtCell(const QPoint &cell, Qt::Alignment alignment, const QPointF &offset)
{
    m_tableView->positionViewAtCell(cell, alignment, offset);
}

QT_END_NAMESPACE
