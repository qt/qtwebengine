/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICKWEBENGINECERTSELECTION_P_H
#define QQUICKWEBENGINECERTSELECTION_P_H

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

#include <QtWebEngine/private/qtwebengineglobal_p.h>

#include <QtCore/QDateTime>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QUrl>
#include <QtCore/QVector>
#include <QtQml/QQmlListProperty>

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)

QT_BEGIN_NAMESPACE

class ClientCertSelectController;
class QQuickWebEngineClientCertificateSelection;

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineClientCertificateOption : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString issuer READ issuer CONSTANT FINAL)
    Q_PROPERTY(QString subject READ subject CONSTANT FINAL)
    Q_PROPERTY(QDateTime effectiveDate READ effectiveDate CONSTANT FINAL)
    Q_PROPERTY(QDateTime expiryDate READ expiryDate CONSTANT FINAL)
    Q_PROPERTY(bool isSelfSigned READ isSelfSigned CONSTANT FINAL)

public:
    QString issuer() const;
    QString subject() const;
    QDateTime effectiveDate() const;
    QDateTime expiryDate() const;
    bool isSelfSigned() const;

    Q_INVOKABLE void select();

private:
    friend class QQuickWebEngineClientCertificateSelection;
    QQuickWebEngineClientCertificateOption(QQuickWebEngineClientCertificateSelection *selection, int index);

    QQuickWebEngineClientCertificateSelection *m_selection;
    int m_index;
};

class Q_WEBENGINE_PRIVATE_EXPORT QQuickWebEngineClientCertificateSelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl host READ host CONSTANT FINAL)
    Q_PROPERTY(QQmlListProperty<QQuickWebEngineClientCertificateOption> certificates READ certificates CONSTANT FINAL)

public:
    QQuickWebEngineClientCertificateSelection() = default;

    QUrl host() const;

    Q_INVOKABLE void select(int idx);
    Q_INVOKABLE void select(const QQuickWebEngineClientCertificateOption *certificate);
    Q_INVOKABLE void selectNone();
    QQmlListProperty<QQuickWebEngineClientCertificateOption> certificates();

private:
    friend class QQuickWebEngineViewPrivate;
    friend class QQuickWebEngineClientCertificateOption;

    static int certificates_count(QQmlListProperty<QQuickWebEngineClientCertificateOption> *p);
    static QQuickWebEngineClientCertificateOption *certificates_at(QQmlListProperty<QQuickWebEngineClientCertificateOption> *p, int idx);

    explicit QQuickWebEngineClientCertificateSelection(QSharedPointer<ClientCertSelectController>);

    mutable QVector<QQuickWebEngineClientCertificateOption *> m_certificates;
    QSharedPointer<ClientCertSelectController> d_ptr;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QQuickWebEngineClientCertificateOption *)
Q_DECLARE_METATYPE(QQmlListProperty<QQuickWebEngineClientCertificateOption>)
Q_DECLARE_METATYPE(QQuickWebEngineClientCertificateSelection *)

#endif

#endif // QQUICKWEBENGINECERTSELECTION_P_H
