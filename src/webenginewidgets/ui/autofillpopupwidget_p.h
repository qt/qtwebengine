// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AUTOFILLPOPUPWIDGET_P_H
#define AUTOFILLPOPUPWIDGET_P_H

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

#include <QFrame>

namespace QtWebEngineCore {
class AutofillPopupController;
}

QT_BEGIN_NAMESPACE
class QListView;
class QWebEngineView;
class QWebEngineViewPrivate;
QT_END_NAMESPACE

namespace QtWebEngineWidgetUI {

// Based on QComboBoxPrivateContainer
class AutofillPopupWidget : public QFrame
{
    Q_OBJECT
public:
    AutofillPopupWidget(QtWebEngineCore::AutofillPopupController *controller,
                        QWebEngineView *parent);
    ~AutofillPopupWidget();

    void showPopup(QPoint pos, int width, bool autoselectFirstSuggestion);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    QtWebEngineCore::AutofillPopupController *m_controller;
    QWebEngineView *m_webEngineView;
    QListView *m_listView;

    friend class QT_PREPEND_NAMESPACE(QWebEngineViewPrivate);
};

} // namespace QtWebEngineWidgetUI

#endif // AUTOFILLPOPUPWIDGET_P_H
