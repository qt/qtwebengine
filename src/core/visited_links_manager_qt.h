// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#ifndef VISITED_LINKS_MANAGER_QT_H
#define VISITED_LINKS_MANAGER_QT_H

#include "qtwebenginecoreglobal_p.h"
#include <QList>
#include <QScopedPointer>

QT_FORWARD_DECLARE_CLASS(QUrl)

namespace visitedlink {
class VisitedLinkWriter;
}

class GURL;

namespace QtWebEngineCore {

class ProfileQt;
class VisitedLinkDelegateQt;

class Q_WEBENGINECORE_PRIVATE_EXPORT VisitedLinksManagerQt {

public:
    virtual~VisitedLinksManagerQt();
    VisitedLinksManagerQt(ProfileQt *profile, bool persistVisitedLinks);

    void deleteAllVisitedLinkData();
    void deleteVisitedLinkDataForUrls(const QList<QUrl> &);

    bool containsUrl(const QUrl &) const;

private:
    void addUrl(const GURL &);
    friend class WebContentsDelegateQt;

    QScopedPointer<visitedlink::VisitedLinkWriter> m_visitedLinkWriter;
    QScopedPointer<VisitedLinkDelegateQt> m_delegate;
};

} // namespace QtWebEngineCore

#endif // WEB_ENGINE_VISITED_LINKS_MANAGER_H
