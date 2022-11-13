// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef PREF_SERVICE_ADAPTER_H
#define PREF_SERVICE_ADAPTER_H

#include "components/prefs/pref_service.h"
#include "qtwebenginecoreglobal_p.h"

namespace QtWebEngineCore {

class ProfileAdapter;

// PrefServiceAdapter manages the collection of tunable preferences.
// Any new preference should be defined and register in the registry
// before it can be used
class PrefServiceAdapter
{
public:

    PrefServiceAdapter() = default;

    void setup(const ProfileAdapter &adapter);
    void commit();
    PrefService *prefService();
    const PrefService *prefService() const;
    std::string mediaDeviceIdSalt() const;

#if QT_CONFIG(webengine_spellchecker)
    void setSpellCheckLanguages(const QStringList &languages);
    QStringList spellCheckLanguages() const;
    void setSpellCheckEnabled(bool enabled);
    bool isSpellCheckEnabled() const;
#endif // QT_CONFIG(webengine_spellchecker)

private:
    std::unique_ptr<PrefService> m_prefService;
};

}

#endif // PREF_SERVICE_ADAPTER_H
