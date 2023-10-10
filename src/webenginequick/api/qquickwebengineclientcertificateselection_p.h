// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#include <QtWebEngineQuick/private/qtwebenginequickglobal_p.h>

#include <QtCore/qdatetime.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qurl.h>
#include <QtQml/qqmllist.h>
#include <QtQml/qqmlregistration.h>

namespace QtWebEngineCore {
class ClientCertSelectController;
}

QT_BEGIN_NAMESPACE

class QQuickWebEngineClientCertificateSelection;

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineClientCertificateOption : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString issuer READ issuer CONSTANT FINAL)
    Q_PROPERTY(QString subject READ subject CONSTANT FINAL)
    Q_PROPERTY(QDateTime effectiveDate READ effectiveDate CONSTANT FINAL)
    Q_PROPERTY(QDateTime expiryDate READ expiryDate CONSTANT FINAL)
    Q_PROPERTY(bool isSelfSigned READ isSelfSigned CONSTANT FINAL)
    QML_NAMED_ELEMENT(WebEngineClientCertificateOption)
    QML_ADDED_IN_VERSION(1, 9)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")

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

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineClientCertificateSelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl host READ host CONSTANT FINAL)
    Q_PROPERTY(QQmlListProperty<QQuickWebEngineClientCertificateOption> certificates READ certificates CONSTANT FINAL)
    QML_NAMED_ELEMENT(WebEngineClientCertificateSelection)
    QML_ADDED_IN_VERSION(1, 9)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")

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

    static qsizetype certificates_count(QQmlListProperty<QQuickWebEngineClientCertificateOption> *p);
    static QQuickWebEngineClientCertificateOption *certificates_at(QQmlListProperty<QQuickWebEngineClientCertificateOption> *p, qsizetype idx);

    explicit QQuickWebEngineClientCertificateSelection(
            QSharedPointer<QtWebEngineCore::ClientCertSelectController>);

    mutable QList<QQuickWebEngineClientCertificateOption *> m_certificates;
    QSharedPointer<QtWebEngineCore::ClientCertSelectController> d_ptr;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QQuickWebEngineClientCertificateOption *)
Q_DECLARE_METATYPE(QQmlListProperty<QQuickWebEngineClientCertificateOption>)
Q_DECLARE_METATYPE(QQuickWebEngineClientCertificateSelection *)

#endif // QQUICKWEBENGINECERTSELECTION_P_H
