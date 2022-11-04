// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINESCRIPTCOLLECTION_P_H
#define QWEBENGINESCRIPTCOLLECTION_P_H

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

#include "qtwebenginecoreglobal.h"
#include "qwebenginescript.h"
#include "web_contents_adapter.h"

#include <QtCore/QList>
#include <QtCore/QSharedPointer>

namespace QtWebEngineCore {
class UserResourceControllerHost;
} // namespace

QT_BEGIN_NAMESPACE
class Q_WEBENGINECORE_PRIVATE_EXPORT QWebEngineScriptCollectionPrivate
{
public:
    QWebEngineScriptCollectionPrivate(QtWebEngineCore::UserResourceControllerHost *, QSharedPointer<QtWebEngineCore::WebContentsAdapter> = QSharedPointer<QtWebEngineCore::WebContentsAdapter>());
    int count() const;
    bool contains(const QWebEngineScript &) const;
    QList<QWebEngineScript> toList(const QString &scriptName = QString()) const;
    void initializationFinished(QSharedPointer<QtWebEngineCore::WebContentsAdapter> contents);
    void insert(const QWebEngineScript &);
    bool remove(const QWebEngineScript &);
    void clear();
    void reserve(int);

private:
    QtWebEngineCore::UserResourceControllerHost *m_scriptController;
    QSharedPointer<QtWebEngineCore::WebContentsAdapter> m_contents;
    QList<QWebEngineScript> m_scripts;
};

QT_END_NAMESPACE

#endif // QWEBENGINESCRIPTCOLLECTION__PH
