// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwebengineclientcertificateselection_p.h"

#include "client_cert_select_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype WebEngineClientCertificateOption
    //! \instantiates QQuickWebEngineClientCertificateOption
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.9
    \brief Represents a client certificate option.

    \sa {WebEngineClientCertificateSelection::certificates} {WebEngineClientCertificateSelection.certificates}
*/

QQuickWebEngineClientCertificateOption::QQuickWebEngineClientCertificateOption(QQuickWebEngineClientCertificateSelection *selection, int index)
        : QObject(selection), m_selection(selection), m_index(index)
{}

/*!
    \qmlproperty string WebEngineClientCertificateOption::issuer
    \brief The issuer of the certificate.
*/

QString QQuickWebEngineClientCertificateOption::issuer() const
{
    return m_selection->d_ptr->certificates().at(m_index).issuerDisplayName();
}

/*!
    \qmlproperty string WebEngineClientCertificateOption::subject
    \brief The subject of the certificate.
*/
QString QQuickWebEngineClientCertificateOption::subject() const
{
    return m_selection->d_ptr->certificates().at(m_index).subjectDisplayName();
}

/*!
    \qmlproperty datetime WebEngineClientCertificateOption::effectiveDate
    \brief The date and time when the certificate becomes valid.
*/
QDateTime QQuickWebEngineClientCertificateOption::effectiveDate() const
{
    return m_selection->d_ptr->certificates().at(m_index).effectiveDate();
}

/*!
    \qmlproperty datetime WebEngineClientCertificateOption::expiryDate
    \brief The date and time when the certificate becomes invalid.
*/
QDateTime QQuickWebEngineClientCertificateOption::expiryDate() const
{
    return m_selection->d_ptr->certificates().at(m_index).expiryDate();
}

/*!
    \qmlproperty bool WebEngineClientCertificateOption::isSelfSigned
    \brief Whether the certificate is only self-signed.
*/
bool QQuickWebEngineClientCertificateOption::isSelfSigned() const
{
    return m_selection->d_ptr->certificates().at(m_index).isSelfSigned();
}

/*!
    \qmlmethod void WebEngineClientCertificateOption::select()

    Selects this client certificate option.
*/
void QQuickWebEngineClientCertificateOption::select()
{
    m_selection->select(m_index);
}

/*!
    \qmltype WebEngineClientCertificateSelection
    //! \instantiates QQuickWebEngineClientCertificateSelection
    \inqmlmodule QtWebEngine
    \since QtWebEngine 1.9
    \brief Provides a selection of client certificates.

    When a web site requests an SSL client certificate, and one or more certificates
    are found in the system's client certificate store, this type provides access to
    the certificates to choose from, as well as a method for selecting one.

    The selection is asynchronous. If no certificate is selected and no copy of the
    object is kept alive, loading will continue without a certificate.

    \sa {WebEngineView::selectClientCertificate}{WebEngineView.selectClientCertificate}
*/

QQuickWebEngineClientCertificateSelection::QQuickWebEngineClientCertificateSelection(
        QSharedPointer<QtWebEngineCore::ClientCertSelectController> selectController)
    : QObject(), d_ptr(selectController)
{}

qsizetype QQuickWebEngineClientCertificateSelection::certificates_count(
        QQmlListProperty<QQuickWebEngineClientCertificateOption> *p)
{
    Q_ASSERT(p && p->object);
    QQuickWebEngineClientCertificateSelection *d = static_cast<QQuickWebEngineClientCertificateSelection *>(p->object);
    return d->m_certificates.size();
}

QQuickWebEngineClientCertificateOption *QQuickWebEngineClientCertificateSelection::certificates_at(
        QQmlListProperty<QQuickWebEngineClientCertificateOption> *p, qsizetype idx)
{
    Q_ASSERT(p && p->object);
    QQuickWebEngineClientCertificateSelection *d = static_cast<QQuickWebEngineClientCertificateSelection *>(p->object);
    if (idx < 0 || idx >= d->m_certificates.size())
        return nullptr;
    return d->m_certificates[idx];
}

/*!
    \qmlproperty list<WebEngineClientCertificateOption> WebEngineClientCertificateSelection::certificates
    \brief The client certificates available to choose from.
*/

QQmlListProperty<QQuickWebEngineClientCertificateOption> QQuickWebEngineClientCertificateSelection::certificates()
{
    if (m_certificates.empty()) {
        QList<QSslCertificate> certificates = d_ptr->certificates();
        for (int i = 0; i < certificates.size(); ++i)
            m_certificates.push_back(new QQuickWebEngineClientCertificateOption(this, i));
    }

    return QQmlListProperty<QQuickWebEngineClientCertificateOption>(
                this, nullptr,
                certificates_count,
                certificates_at);
}

/*!
    \qmlmethod void WebEngineClientCertificateSelection::select(WebEngineClientCertificateOption certificate)

    Selects the client certificate \a certificate. The certificate must be one
    of the offered certificates.

    \sa selectNone()
*/
void QQuickWebEngineClientCertificateSelection::select(const QQuickWebEngineClientCertificateOption *certificate)
{
    select(certificate->m_index);
}

/*!
    \qmlmethod void WebEngineClientCertificateSelection::select(int index)

    Selects the client certificate at the index \a index in the list of offered certificates.

    \sa selectNone()
*/
void QQuickWebEngineClientCertificateSelection::select(int index)
{
    d_ptr->select(index);
}

/*!
    \qmlmethod void WebEngineClientCertificateSelection::selectNone()

    Continues without using any of the offered certificates. This is the same
    action as taken when destroying the last copy of this object if no
    selection has been made.

    \sa select()
*/
void QQuickWebEngineClientCertificateSelection::selectNone()
{
    d_ptr->selectNone();
}

/*!
    \qmlproperty url WebEngineClientCertificateSelection::host
    \brief The host and port of the server requesting the client certificate.
*/
QUrl QQuickWebEngineClientCertificateSelection::host() const
{
    return d_ptr->hostAndPort();
}

QT_END_NAMESPACE

#include "moc_qquickwebengineclientcertificateselection_p.cpp"
