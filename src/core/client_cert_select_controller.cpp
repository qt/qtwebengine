// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "client_cert_select_controller.h"

#include <base/functional/bind.h>
#include <content/public/browser/client_certificate_delegate.h>
#include <net/cert/x509_certificate.h>
#include <net/ssl/client_cert_identity.h>
#include <net/ssl/ssl_cert_request_info.h>
#include <net/ssl/ssl_info.h>
#include "net/ssl/ssl_private_key.h"

#include "type_conversion.h"

#include <QDebug>

namespace QtWebEngineCore {

ClientCertSelectController::ClientCertSelectController(net::SSLCertRequestInfo *certRequestInfo,
                                                       std::vector<std::unique_ptr<net::ClientCertIdentity>> clientCerts,
                                                       std::unique_ptr<content::ClientCertificateDelegate> delegate)
            : m_clientCerts(std::move(clientCerts))
            , m_delegate(std::move(delegate))
            , m_selected(false)
{
    m_hostAndPort.setHost(QString::fromStdString(certRequestInfo->host_and_port.HostForURL()));
    m_hostAndPort.setPort(certRequestInfo->host_and_port.port());
}

ClientCertSelectController::~ClientCertSelectController()
{
    // Continue without a client certificate, for instance if the app has not
    // implemented support for client certificate selection.
    if (!m_selected)
        m_delegate->ContinueWithCertificate(nullptr, nullptr);
}

void ClientCertSelectController::selectNone()
{
    if (m_selected) {
        LOG(WARNING) << "ClientCertSelectController::selectNone() certificate already selected";
        return;
    }
    m_selected = true;
    m_delegate->ContinueWithCertificate(nullptr, nullptr);
}

void ClientCertSelectController::select(int index)
{
    if (m_selected) {
        LOG(WARNING) << "ClientCertSelectController::select() certificate already selected";
        return;
    }
    for (auto &certInfo : m_clientCerts) {
        if (index == 0) {
            m_selected = true;
            scoped_refptr<net::X509Certificate> cert = certInfo->certificate();
            net::ClientCertIdentity::SelfOwningAcquirePrivateKey(
                        std::move(certInfo),
                        base::BindOnce(&content::ClientCertificateDelegate::ContinueWithCertificate,
                                       std::move(m_delegate), std::move(cert)));
            return;
        }
        std::vector<std::string> pem_encoded;
        if (certInfo->certificate()->GetPEMEncodedChain(&pem_encoded))
            --index;
    }
    LOG(WARNING) << "ClientCertSelectController::select() index out of range:" << index;
}

void ClientCertSelectController::select(const QSslCertificate &certificate)
{
    if (m_selected) {
        LOG(WARNING) << "ClientCertSelectController::select() certificate already selected";
        return;
    }
    QByteArray derCertificate = certificate.toDer();
    scoped_refptr<net::X509Certificate> selectedCert =
            net::X509Certificate::CreateFromBytes(base::make_span((const unsigned char *)derCertificate.constData(),
                                                                  (long unsigned)derCertificate.length()));
    for (auto &certInfo : m_clientCerts) {
        scoped_refptr<net::X509Certificate> cert = certInfo->certificate();
        if (cert->EqualsExcludingChain(selectedCert.get())) {
            m_selected = true;
            net::ClientCertIdentity::SelfOwningAcquirePrivateKey(
                        std::move(certInfo),
                        base::BindOnce(&content::ClientCertificateDelegate::ContinueWithCertificate,
                                       std::move(m_delegate), std::move(cert)));
            return;
        }
    }
    LOG(WARNING) << "ClientCertSelectController::select() - selected client certificate not recognized."
                 << "    Selected certificate needs to be one of the offered";
}

QList<QSslCertificate> ClientCertSelectController::certificates() const
{
    if (!m_certificates.isEmpty())
        return m_certificates;
    for (auto &cert : m_clientCerts) {
        std::vector<std::string> pem_encoded;
        if (cert->certificate()->GetPEMEncodedChain(&pem_encoded))
            m_certificates.append(QSslCertificate(QByteArray::fromStdString(pem_encoded.front())));
    }
    return m_certificates;
}

}
