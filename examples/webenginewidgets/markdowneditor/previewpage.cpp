// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "previewpage.h"

#include <QDesktopServices>

bool PreviewPage::acceptNavigationRequest(const QUrl &url,
                                          QWebEnginePage::NavigationType /*type*/,
                                          bool /*isMainFrame*/)
{
    // Only allow qrc:/index.html.
    if (url.scheme() == QString("qrc"))
        return true;
    QDesktopServices::openUrl(url);
    return false;
}
