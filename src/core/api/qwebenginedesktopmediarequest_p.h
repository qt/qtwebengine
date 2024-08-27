// Copyright (C) 2023 The Qt Company Ltd.
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

#ifndef QWEBENGINEDESKTOPMEDIAREQUEST_P_H
#define QWEBENGINEDESKTOPMEDIAREQUEST_P_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/QSharedData>
#include <QtCore/qobject.h>
#include <QtWebEngineCore/qtwebenginecoreglobal.h>

namespace QtWebEngineCore {
class DesktopMediaController;
}

QT_BEGIN_NAMESPACE
class QWebEngineMediaSourceModel;

class QWebEngineDesktopMediaRequestPrivate : public QSharedData
{
public:
    ~QWebEngineDesktopMediaRequestPrivate();
    explicit QWebEngineDesktopMediaRequestPrivate(
            QtWebEngineCore::DesktopMediaController *controller);

    void selectScreen(const QModelIndex &index);
    void selectWindow(const QModelIndex &index);
    void cancel();

    bool didSelectOrCancel = false;
    std::unique_ptr<QtWebEngineCore::DesktopMediaController> controller;
    std::unique_ptr<QWebEngineMediaSourceModel> m_screensModel;
    std::unique_ptr<QWebEngineMediaSourceModel> m_windowsModel;
};

QT_END_NAMESPACE

#endif // QWEBENGINEDESKTOPMEDIAREQUEST_P_H
