/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "widgetwindow.h"

#include "qwebengineview.h"
#include "qwebenginepage.h"
#include "qwebenginehistory.h"
#include "../../common/util.h"

#include <QShortcut>

static const int margin = 1;

WidgetWindow::WidgetWindow()
: m_webView(new QWebEngineView)
, addressLineEdit(0)
{
    setGeometry(0, 0, 800, 600);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(margin, margin, margin, margin);

    // Create a widget based address bar.
    QHBoxLayout* addressBar = new QHBoxLayout;
    addressBar->setSpacing(margin); // Bigger buttons, less space between them

    backButton = new QToolButton;
    backButton->setIcon(QIcon(":/icons/go-previous.png"));
    addressBar->addWidget(backButton);

    forwardButton = new QToolButton;
    forwardButton->setIcon(QIcon(":/icons/go-next.png"));
    addressBar->addWidget(forwardButton);

    reloadButton = new QToolButton;
    reloadButton->setIcon(QIcon::fromTheme(":/icons/view-refresh.png"));
    addressBar->addWidget(reloadButton);

    addressLineEdit =  new QLineEdit;
    addressBar->addWidget(addressLineEdit);

    QShortcut* focusUrlBarShortcut = new QShortcut(addressLineEdit);
    focusUrlBarShortcut->setKey(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(focusUrlBarShortcut, SIGNAL(activated()), addressLineEdit, SLOT(setFocus()));
    connect(focusUrlBarShortcut, SIGNAL(activated()), addressLineEdit, SLOT(selectAll()));

    layout->addLayout(addressBar);
    layout->addWidget(m_webView.data());

    setLayout(layout);

    connect(addressLineEdit, SIGNAL(returnPressed()), SLOT(loadAddressFromAddressBar()));
    connect(backButton, SIGNAL(clicked()), m_webView.data(), SLOT(back()));
    connect(forwardButton, SIGNAL(clicked()), m_webView.data(), SLOT(forward()));
    connect(reloadButton, SIGNAL(clicked()), m_webView.data(), SLOT(reload()));
    connect(m_webView.data(), SIGNAL(loadStarted()), SLOT(loadStarted()));
    connect(m_webView.data(), SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
    connect(m_webView.data(), SIGNAL(titleChanged(const QString&)), SLOT(setWindowTitle(const QString&)));
    connect(m_webView.data(), SIGNAL(urlChanged(const QUrl&)), SLOT(setAddressBarUrl(const QUrl&)));
    connect(m_webView.data(), SIGNAL(fullScreenRequested(bool)), SLOT(fullScreenRequested(bool)));

    m_webView->load(startupUrl());
}

WidgetWindow::~WidgetWindow()
{
}

void WidgetWindow::loadAddressFromAddressBar()
{
    m_webView->load(urlFromUserInput(addressLineEdit->text()));
}

void WidgetWindow::setAddressBarUrl(const QUrl& url)
{
    addressLineEdit->setText(url.toString());
}

void WidgetWindow::loadStarted()
{
    reloadButton->setIcon(QIcon(":/icons/process-stop.png"));
}

void WidgetWindow::loadFinished(bool success)
{
    Q_UNUSED(success);
    forwardButton->setEnabled(m_webView->page()->history()->canGoForward());
    backButton->setEnabled(m_webView->page()->history()->canGoBack());
    reloadButton->setIcon(QIcon(":/icons/view-refresh.png"));
}

class FullScreenParent : public QWidget
{
public:
    FullScreenParent(WidgetWindow *parent)
        : QWidget(parent, Qt::Window)
    {
        setLayout(new QVBoxLayout);
        layout()->setContentsMargins(0, 0, 0, 0);
        setFocusPolicy(Qt::StrongFocus);

        // Make sure the new window pops up at the same position as the view currently has.
        // This is necessary to make the full screen animation on mac look somewhat acceptable.
        QWebEngineView *view = parent->m_webView.data();
        QPoint globalPos = parent->m_webView.data()->mapToGlobal(QPoint(0,0));
        setGeometry(QRect(globalPos, view->size()));
        layout()->addWidget(view);
        showFullScreen();
        // The view must be set to fullscreen as well, as this is what is being checked by Chromium.
        view->showFullScreen();
    }

    ~FullScreenParent()
    {
        // Reparent the view back into it's original position.
        WidgetWindow* originalParent = qobject_cast<WidgetWindow*>(parent());
        originalParent->layout()->addWidget(originalParent->m_webView.data());
    }

protected:
    virtual void changeEvent(QEvent * event) Q_DECL_OVERRIDE
    {
        if (event->type() == QEvent::WindowStateChange
            && !(windowState() & Qt::WindowFullScreen)) {
            qobject_cast<WidgetWindow*>(parent())->fullScreenRequested(false);
        } else
            QWidget::changeEvent(event);
    }

    virtual void keyPressEvent(QKeyEvent * event) Q_DECL_OVERRIDE
    {
        if (event->key() == Qt::Key_Escape)
            qobject_cast<WidgetWindow*>(parent())->fullScreenRequested(false);
        else
            QWidget::keyPressEvent(event);
    }
};

void WidgetWindow::fullScreenRequested(bool fullScreen)
{
    static FullScreenParent* fullScreenParent = 0;
    if (fullScreen) {
        // Create a new parent top level window that can be scaled to full screen.
        fullScreenParent = new FullScreenParent(this);
    } else {
        fullScreenParent->deleteLater();
        fullScreenParent = 0;
    }
}

