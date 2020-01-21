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

#ifndef QQUICKPDFNAVIGATIONSTACK_P_H
#define QQUICKPDFNAVIGATIONSTACK_P_H

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

#include "qquickpdfdocument_p.h"

#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QQuickPdfNavigationStack : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(bool backAvailable READ backAvailable NOTIFY backAvailableChanged)
    Q_PROPERTY(bool forwardAvailable READ forwardAvailable NOTIFY forwardAvailableChanged)

public:
    explicit QQuickPdfNavigationStack(QObject *parent = nullptr);

    Q_INVOKABLE void forward();
    Q_INVOKABLE void back();

    int currentPage() const { return m_currentPage; }
    void setCurrentPage(int currentPage);

    bool backAvailable() const;
    bool forwardAvailable() const;

Q_SIGNALS:
    void currentPageChanged();
    void currentPageJumped(int page);
    void backAvailableChanged();
    void forwardAvailableChanged();

private:
    QVector<int> m_pageHistory;
    int m_nextHistoryIndex = 0;
    int m_currentPage = 0;
    bool m_changing = false;

    Q_DISABLE_COPY(QQuickPdfNavigationStack)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickPdfNavigationStack)

#endif // QQUICKPDFNAVIGATIONSTACK_P_H
