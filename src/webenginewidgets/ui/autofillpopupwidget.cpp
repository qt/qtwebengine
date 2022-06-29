// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "autofillpopupwidget_p.h"
#include "qwebengineview.h"
#include "qwebengineview_p.h"

#include "autofill_popup_controller.h"

#include <QApplication>
#include <QBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QListView>
#include <QMouseEvent>

namespace QtWebEngineWidgetUI {

AutofillPopupWidget::AutofillPopupWidget(QtWebEngineCore::AutofillPopupController *controller,
                                         QWebEngineView *parent)
    : QFrame(parent, Qt::Popup), m_controller(controller), m_webEngineView(parent)
{
    setAttribute(Qt::WA_WindowPropagation);
    setAttribute(Qt::WA_X11NetWmWindowTypeCombo);

    // we need a vertical layout
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());

    m_listView = new QListView(m_webEngineView);
    m_listView->setModel(m_controller->model());
    m_listView->setTextElideMode(Qt::ElideMiddle);

    // Based on QComboBoxPrivateContainer::setItemView
    m_listView->setParent(this);
    m_listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    layout->insertWidget(0, m_listView);
    m_listView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    m_listView->installEventFilter(this);
    // Necessary for filtering QEvent::MouseMove:
    m_listView->viewport()->installEventFilter(this);

    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // TODO: Implement vertical scrollbar. Chromium also has it.
    m_listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setMouseTracking(true);

    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Some styles (Mac) have a margin at the top and bottom of the popup.
    layout->insertSpacing(0, 0);
    layout->addSpacing(0);

    connect(m_controller, &QtWebEngineCore::AutofillPopupController::currentIndexChanged,
            m_listView, &QListView::setCurrentIndex);
}

AutofillPopupWidget::~AutofillPopupWidget() { }

// Based on QComboBox::showPopup()
void AutofillPopupWidget::showPopup(QPoint pos, int width, bool autoselectFirstSuggestion)
{
    QStyle *const style = m_webEngineView->style();
    QStyleOptionComboBox opt;
    opt.initFrom(m_webEngineView);

    if (autoselectFirstSuggestion)
        m_controller->selectFirstSuggestion();

    QRect listRect(pos, QSize(width, 0));

    // Calculate height
    {
        int listHeight = 0;
        int rowCount = m_controller->model()->rowCount();
        for (int i = 0; i < rowCount; ++i) {
            QModelIndex idx = m_controller->model()->index(i, 0);
            listHeight += m_listView->visualRect(idx).height();
        }
        if (rowCount > 1)
            listHeight += (rowCount - 1) * m_listView->spacing() * 2;

        listRect.setHeight(listRect.height() + listHeight);
    }

    // Calculate height margin
    {
        int heightMargin = m_listView->spacing() * 2;

        // Add the frame of the popup
        const QMargins pm = contentsMargins();
        heightMargin += pm.top() + pm.bottom();

        // Add the frame of the list view
        const QMargins vm = m_listView->contentsMargins();
        heightMargin += vm.top() + vm.bottom();
        listRect.setHeight(listRect.height() + heightMargin);
    }

    // Add space for margin at top and bottom if the style wants it
    int styleMargin = style->pixelMetric(QStyle::PM_MenuVMargin, &opt, this) * 2;
    listRect.setHeight(listRect.height() + styleMargin);

    // Takes account of the mimium/maximum size of the popup
    layout()->activate();
    listRect.setSize(listRect.size().expandedTo(minimumSize()).boundedTo(maximumSize()));

    setGeometry(listRect);
    QFrame::show();
}

bool AutofillPopupWidget::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseMove:
        if (isVisible()) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QModelIndex indexUnderMouse = m_listView->indexAt(mouseEvent->position().toPoint());
            if (indexUnderMouse.isValid()
                && indexUnderMouse.data(Qt::AccessibleDescriptionRole).toString()
                        != QLatin1String("separator")) {
                m_controller->selectSuggestion(indexUnderMouse.row());
            }
        }
        return true;
    case QEvent::MouseButtonRelease: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            m_controller->acceptSuggestion();
            return true;
        }
        break;
    }
    default:
        break;
    }

    return QFrame::eventFilter(object, event);
}

void AutofillPopupWidget::keyPressEvent(QKeyEvent *event)
{
    // AutofillPopupControllerImpl::HandleKeyPressEvent()
    // chrome/browser/ui/autofill/autofill_popup_controller_impl.cc
    switch (event->key()) {
    case Qt::Key_Up:
        m_controller->selectPreviousSuggestion();
        return;
    case Qt::Key_Down:
        m_controller->selectNextSuggestion();
        return;
    case Qt::Key_PageUp:
        m_controller->selectFirstSuggestion();
        return;
    case Qt::Key_PageDown:
        m_controller->selectLastSuggestion();
        return;
    case Qt::Key_Escape:
        m_webEngineView->d_ptr->hideAutofillPopup();
        return;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        m_controller->acceptSuggestion();
        return;
    case Qt::Key_Delete:
        // Remove suggestion is not supported for datalist.
        // Forward delete to view to be able to remove selected text.
        break;
    case Qt::Key_Tab:
        m_controller->acceptSuggestion();
        break;
    default:
        break;
    }

    QCoreApplication::sendEvent(m_webEngineView->focusWidget(), event);
}

void AutofillPopupWidget::keyReleaseEvent(QKeyEvent *event)
{
    // Do not forward release events of the overridden key presses.
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Escape:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        return;
    default:
        break;
    }

    QCoreApplication::sendEvent(m_webEngineView->focusWidget(), event);
}

} // namespace QtWebEngineWidgetUI
