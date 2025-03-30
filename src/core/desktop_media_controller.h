// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DESKTOP_MEDIA_CONTROLLER_H
#define DESKTOP_MEDIA_CONTROLLER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QtCore/QObject>

#include <QString>

namespace QtWebEngineCore {
class DesktopMediaListQtPrivate;
class DesktopMediaControllerPrivate;

enum DesktopMediaType { Screen = 0, Window };

class Q_WEBENGINECORE_EXPORT DesktopMediaListQt : public QObject
{
    Q_OBJECT
public:
    ~DesktopMediaListQt() override;

    QString getSourceName(int index) const;
    int getSourceCount() const;

Q_SIGNALS:
    void initialized();
    void itemSelected(int index);
    void sourceAdded(int index);
    void sourceRemoved(int index);
    void sourceMoved(int oldIndex, int newIndex);
    void sourceNameChanged(int index);

private:
    friend class DesktopMediaController;
    friend class DesktopMediaControllerPrivate;
    explicit DesktopMediaListQt(DesktopMediaType type);
    std::unique_ptr<DesktopMediaListQtPrivate> d;
};

class Q_WEBENGINECORE_EXPORT DesktopMediaController : public QObject
{
    Q_OBJECT
public:
    explicit DesktopMediaController(DesktopMediaControllerPrivate *dd);
    ~DesktopMediaController() override;

    DesktopMediaListQt *screens() const;
    DesktopMediaListQt *windows() const;

    void selectScreen(int index);
    void selectWindow(int index);
    void cancel();

Q_SIGNALS:
    void mediaListsInitialized();

private:
    std::unique_ptr<DesktopMediaControllerPrivate> d;
};

} // namespace QtWebEngineCore
#endif // DESKTOP_MEDIA_CONTROLLER_H
