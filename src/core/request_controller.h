// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef REQUEST_CONTROLLER_H
#define REQUEST_CONTROLLER_H

#include "qtwebenginecoreglobal.h"

#include <QUrl>

namespace QtWebEngineCore {

class RequestController {
public:
    RequestController(QUrl origin)
        : m_answered(false)
        , m_origin(std::move(origin))
    {}

    QUrl origin() const { return m_origin; }

    void accept()
    {
        if (!m_answered) {
            m_answered = true;
            accepted();
        }
    }

    void reject()
    {
        if (!m_answered) {
            m_answered = true;
            rejected();
        }
    }

    virtual ~RequestController() {}

protected:
    virtual void accepted() = 0;
    virtual void rejected() = 0;

private:
    bool m_answered;
    QUrl m_origin;
};

} // namespace QtWebEngineCore

#endif // !REQUEST_CONTROLLER_H
