// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QRC_URL_SCHEME_HANDLER_H
#define QRC_URL_SCHEME_HANDLER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QtWebEngineCore/qwebengineurlschemehandler.h>

namespace QtWebEngineCore {

class QrcUrlSchemeHandler final : public QWebEngineUrlSchemeHandler
{
public:
    void requestStarted(QWebEngineUrlRequestJob *) override;
};

} // namespace QtWebEngineCore

#endif // !QRC_URL_SCHEME_HANDLER_H
