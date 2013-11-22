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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSizePolicy>

#include "authentication_dialog_qt.h"

namespace {
    const QString stylesheet = "QWidget {"
        "background-color: white;\n"
        "border-top-left-radius: 12px;\n"
        "border-top-right-radius: 12px;\n"
        "border-bottom-left-radius: 12px;\n"
        "border-bottom-right-radius: 12px;\n"
        "}\n"
        "\n"
        "QLabel#lblTitle {\n"
        "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #0073bc, stop:1 #00a8df);\n"
        "color: #fafafa;\n"
        "padding-left: 10px;\n"
        "padding-right: 10px;\n"
        "border-bottom-left-radius: 0px;\n"
        "border-bottom-right-radius: 0px;\n"
        "}\n"
        "\n"
        "QPushButton {\n"
        "background-color: rgb(250, 250, 250);\n"
        "border: 1px solid grey;\n"
        "}\n"
        "\n"
        "QDialog \n"
        "{\n"
        "  background-color: rgb(0,0,0, 50);\n"
        "}\n"
        "\n"
        "QLabel#lblMsg\n"
        "{\n"
        "  margin-left: 10px;\n"
        "  margin-right: 10px;\n"
        "  border-radius: 0px;\n"
        "}\n"
        "\n"
        "QLineEdit\n"
        "{\n"
        "  margin-left: 10px;\n"
        "  margin-right: 10px;\n"
        "  border: 1px solid grey;\n"
        "  border-radius: 2px;\n"
        "}";
}

AuthenticationPopupDialog::AuthenticationPopupDialog(const QString &host, const QString &realm, QWidget *parent)
        : QDialog(parent)
{
    QVBoxLayout *verticalLayoutDialog;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QLabel *lblTitle;
    QLabel *lblMsg;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnOk;
    QPushButton *btnCancel;
    QSpacerItem *verticalSpacer_dialog;

    this->setObjectName(QStringLiteral("authenticationDialog"));
    this->setStyleSheet(stylesheet);

    verticalLayoutDialog = new QVBoxLayout(this);
    verticalLayoutDialog->setObjectName(QStringLiteral("verticalLayoutDialog"));
    verticalLayoutDialog->setContentsMargins(30, 100, 30, -1);
    widget = new QWidget(this);
    widget->setObjectName(QStringLiteral("widget"));
    verticalLayout = new QVBoxLayout(widget);
    verticalLayout->setSpacing(15);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 15);
    lblTitle = new QLabel(widget);
    lblTitle->setObjectName(QStringLiteral("lblTitle"));
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(lblTitle->sizePolicy().hasHeightForWidth());
    lblTitle->setSizePolicy(sizePolicy1);
    lblTitle->setMinimumSize(QSize(0, 93));
    lblTitle->setMaximumSize(QSize(16777215, 93));

    verticalLayout->addWidget(lblTitle);

    lblMsg = new QLabel(widget);
    lblMsg->setObjectName(QStringLiteral("lblMsg"));
    lblMsg->setMinimumSize(QSize(0, 60));
    lblMsg->setMaximumSize(QSize(16777215, 120));
    lblMsg->setWordWrap(true);

    verticalLayout->addWidget(lblMsg);

    m_userEdit = new QLineEdit(widget);
    m_userEdit->setObjectName(QStringLiteral("leUser"));
    m_userEdit->setMinimumSize(QSize(0, 79));
    m_userEdit->setMaximumSize(QSize(16777215, 79));

    verticalLayout->addWidget(m_userEdit);

    m_passwordEdit = new QLineEdit(widget);
    m_passwordEdit->setObjectName(QStringLiteral("lePwd"));
    m_passwordEdit->setMinimumSize(QSize(0, 79));
    m_passwordEdit->setMaximumSize(QSize(16777215, 79));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    verticalLayout->addWidget(m_passwordEdit);

    verticalSpacer = new QSpacerItem(20, 15, QSizePolicy::Minimum, QSizePolicy::Fixed);

    verticalLayout->addItem(verticalSpacer);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(15);
    horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
    horizontalLayout->setContentsMargins(10, -1, 10, -1);
    btnOk = new QPushButton(widget);
    btnOk->setObjectName(QStringLiteral("btnOk"));
    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(btnOk->sizePolicy().hasHeightForWidth());
    btnOk->setSizePolicy(sizePolicy2);
    btnOk->setMinimumSize(QSize(0, 79));
    btnOk->setMaximumSize(QSize(16777215, 79));

    btnCancel = new QPushButton(widget);
    btnCancel->setObjectName(QStringLiteral("btnCancel"));
    sizePolicy2.setHeightForWidth(btnCancel->sizePolicy().hasHeightForWidth());
    btnCancel->setSizePolicy(sizePolicy2);
    btnCancel->setMinimumSize(QSize(0, 79));
    btnCancel->setMaximumSize(QSize(16777215, 79));
    btnCancel->setStyleSheet(QStringLiteral(""));

    horizontalLayout->addWidget(btnCancel);
    horizontalLayout->addWidget(btnOk);

    verticalLayout->addLayout(horizontalLayout);

    verticalLayoutDialog->addWidget(widget);

    verticalSpacer_dialog = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayoutDialog->addItem(verticalSpacer_dialog);

    QObject::connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    QObject::connect(btnOk, SIGNAL(clicked()), this, SLOT(accept()));

    lblTitle->setText(tr("Authentication Required"));
    QString message = tr("The server ");
    message += host;
    message += tr(" requires a username and password.");
    message += tr(" The server says: ");
    message += realm;
    message += ".";
    lblMsg->setText(message);
    btnOk->setText(tr("Ok"));
    btnCancel->setText(tr("Cancel"));
    m_userEdit->setPlaceholderText(tr("Username"));
    m_passwordEdit->setPlaceholderText(tr("Password"));
}

AuthenticationPopupDialog::~AuthenticationPopupDialog()
{
    // TODO Auto-generated destructor stub
}

void AuthenticationPopupDialog::authenticationConfirmed(QString &username, QString &password) const
{
    username = m_userEdit->text().trimmed();
    password = m_passwordEdit->text().trimmed();
}
