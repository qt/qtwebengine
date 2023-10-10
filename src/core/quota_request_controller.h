// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QUOTA_REQUEST_CONTROLLER_H
#define QUOTA_REQUEST_CONTROLLER_H

#include "request_controller.h"

namespace QtWebEngineCore {

class QuotaRequestController : public RequestController {
public:
    QuotaRequestController(QUrl origin, qint64 requestedSize)
        : RequestController(std::move(origin))
        , m_requestedSize(requestedSize)
    {}

    qint64 requestedSize() const { return m_requestedSize; }

private:
    qint64 m_requestedSize;
};

} // namespace QtWebEngineCore

#endif // QUOTA_REQUEST_CONTROLLER_H
