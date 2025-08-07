// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtwebenginecoreglobal_p.h"

#include "web_engine_context.h"
#include "web_engine_library_info.h"

#include "base/base_paths.h"
#include "base/i18n/icu_util.h"
#include "base/path_service.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"

#include <QUrl>

namespace QtWebEngineCore {
Q_WEBENGINECORE_EXPORT void initialize() { }

bool closingDown()
{
    return WebEngineContext::closingDown();
}

} // namespace QtWebEngineCore

#if defined(Q_OS_WIN)
namespace QtWebEngineSandbox {
sandbox::SandboxInterfaceInfo *staticSandboxInterfaceInfo(sandbox::SandboxInterfaceInfo *info)
{
    static sandbox::SandboxInterfaceInfo *g_info = nullptr;
    if (info) {
        Q_ASSERT(g_info == nullptr);
        g_info = info;
    }
    return g_info;
}
} //namespace
#endif

QT_BEGIN_NAMESPACE

QString qWebEngineGetDomainAndRegistry(const QUrl &url) {
    base::FilePath icuDataPath;
    // Let's assume that ICU is already initialized if DIR_QT_LIBRARY_DATA is set.
    if (!base::PathService::Get(base::DIR_QT_LIBRARY_DATA, &icuDataPath)) {
        icuDataPath = WebEngineLibraryInfo::getPath(base::DIR_QT_LIBRARY_DATA);
        if (!base::PathService::OverrideAndCreateIfNeeded(base::DIR_QT_LIBRARY_DATA, icuDataPath, false, false))
            qWarning("Failed to set ICU data path.");
        base::i18n::InitializeICU();
    }

    const QString host = url.host();
    const std::string domain = net::registry_controlled_domains::GetDomainAndRegistry(host.toStdString(), net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
    return QString::fromStdString(domain);
}

QT_END_NAMESPACE
