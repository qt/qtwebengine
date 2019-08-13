
/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef PREF_SERVICE_ADAPTER_H
#define PREF_SERVICE_ADAPTER_H

#include "components/prefs/pref_service.h"
#include "qtwebenginecoreglobal_p.h"

QT_BEGIN_NAMESPACE
class QStringList;
QT_END_NAMESPACE

class ProfileAdapter;

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
