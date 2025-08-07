// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBENGINEEXTENSION_P_H_
#define QWEBENGINEEXTENSION_P_H_

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

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#if QT_CONFIG(webengine_extensions)
#include <QtCore/qsharedpointer.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>

namespace QtWebEngineCore {
class ExtensionManager;
}

QT_BEGIN_NAMESPACE

class QWebEngineExtensionInfoPrivate : public QSharedData
{
public:
    struct ExtensionData
    {
        std::string id;
        QString name;
        QString description;
        QString path;
        QString error;
        QUrl actionPopupUrl;
    };

    QWebEngineExtensionInfoPrivate(const ExtensionData &data,
                                   QtWebEngineCore::ExtensionManager *manager);
    ~QWebEngineExtensionInfoPrivate();
    std::string id() const;
    QString name() const;
    QString description() const;
    QString path() const;
    QString error() const;
    QUrl actionPopupUrl() const;
    bool isEnabled() const;
    bool isLoaded() const;
    bool isInstalled() const;

private:
    ExtensionData m_data;
    QtWebEngineCore::ExtensionManager *m_manager;
};

QT_END_NAMESPACE

#endif // QT_CONFIG(webengine_extensions)
#endif // QWEBENGINEEXTENSION_P_H_
