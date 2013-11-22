/*
 *  Copyright (C) 2013 BlackBerry Limited. All rights reserved.
 */

#ifndef AUTHENTICATION_DIALOG_QT_H_
#define AUTHENTICATION_DIALOG_QT_H_

#include <QDialog>

class QLineEdit;

class AuthenticationPopupDialog: public QDialog
{
public:
    AuthenticationPopupDialog(const QString &message, QWidget *parent);
    virtual ~AuthenticationPopupDialog();

    void authenticationConfirmed(QString &username, QString &password) const;

private:
    QLineEdit *m_userEdit;
    QLineEdit *m_passwordEdit;
};

#endif /* AUTHENTICATION_DIALOG_QT_H_ */
