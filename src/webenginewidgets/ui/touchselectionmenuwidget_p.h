// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef TOUCHSELECTIONMENUWIDGET_P_H
#define TOUCHSELECTIONMENUWIDGET_P_H

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

#include <QPushButton>
#include <QWidget>

namespace QtWebEngineCore {
class TouchSelectionMenuController;
}

QT_BEGIN_NAMESPACE
class QWebEngineView;
QT_END_NAMESPACE

namespace QtWebEngineWidgetUI {
class TouchSelectionMenuWidget : public QWidget
{
public:
    class TouchButton : public QPushButton
    {
    public:
        TouchButton(QString name, QWidget *parent);
        ~TouchButton();

    protected:
        bool event(QEvent *ev) override;
    };

    TouchSelectionMenuWidget(QWebEngineView *view,
                             QtWebEngineCore::TouchSelectionMenuController *controller);
    ~TouchSelectionMenuWidget();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
};
} // namespace QtWebEngineWidgetUI

#endif // TOUCHSELECTIONMENUWIDGET_P_H
