// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWEBENGINDIALOGREQUESTS_P_H
#define QQUICKWEBENGINDIALOGREQUESTS_P_H

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

#include <QtWebEngineQuick/private/qtwebenginequickglobal_p.h>
#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qurl.h>
#include <QtGui/qcolor.h>
#include <QtQml/qqmlregistration.h>

namespace QtWebEngineCore {
    class AuthenticationDialogController;
    class ColorChooserController;
    class FilePickerController;
    class JavaScriptDialogController;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineAuthenticationDialogRequest : public QObject {
    Q_OBJECT
public:

    enum AuthenticationType {
        AuthenticationTypeHTTP,
        AuthenticationTypeProxy
    };

    Q_ENUM(AuthenticationType)

    Q_PROPERTY(QUrl url READ url CONSTANT FINAL)
    Q_PROPERTY(QString realm READ realm CONSTANT FINAL)
    Q_PROPERTY(QString proxyHost READ proxyHost CONSTANT FINAL)
    Q_PROPERTY(AuthenticationType type READ type CONSTANT FINAL)
    Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted FINAL)
    QML_NAMED_ELEMENT(AuthenticationDialogRequest)
    QML_ADDED_IN_VERSION(1, 4)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")

    ~QQuickWebEngineAuthenticationDialogRequest();

    QUrl url() const;
    QString realm() const;
    QString proxyHost() const;
    AuthenticationType type() const;
    bool isAccepted() const;
    void setAccepted(bool accepted);

public slots:
    void dialogAccept(const QString &user, const QString &password);
    void dialogReject();

private:
    QQuickWebEngineAuthenticationDialogRequest(QSharedPointer<QtWebEngineCore::AuthenticationDialogController> controller,
                                    QObject *parent = nullptr);
    QWeakPointer<QtWebEngineCore::AuthenticationDialogController> m_controller;
    QUrl m_url;
    QString m_realm;
    AuthenticationType m_type;
    QString m_host;
    bool m_accepted;
    friend class QQuickWebEngineViewPrivate;
    Q_DISABLE_COPY(QQuickWebEngineAuthenticationDialogRequest)
};

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineJavaScriptDialogRequest : public QObject {
    Q_OBJECT
public:

    enum DialogType {
        DialogTypeAlert,
        DialogTypeConfirm,
        DialogTypePrompt,
        DialogTypeBeforeUnload,
    };
    Q_ENUM(DialogType)

    Q_PROPERTY(QString message READ message CONSTANT FINAL)
    Q_PROPERTY(QString defaultText READ defaultText CONSTANT FINAL)
    Q_PROPERTY(QString title READ title CONSTANT FINAL)
    Q_PROPERTY(DialogType type READ type CONSTANT FINAL)
    Q_PROPERTY(QUrl securityOrigin READ securityOrigin CONSTANT FINAL)
    Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted FINAL)
    QML_NAMED_ELEMENT(JavaScriptDialogRequest)
    QML_ADDED_IN_VERSION(1, 4)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")

    ~QQuickWebEngineJavaScriptDialogRequest();

    QString message() const;
    QString defaultText() const;
    QString title() const;
    DialogType type() const;
    QUrl securityOrigin() const;
    bool isAccepted() const;
    void setAccepted(bool accepted);

public slots:
    void dialogAccept(const QString& text = QString());
    void dialogReject();

private:
    QQuickWebEngineJavaScriptDialogRequest(QSharedPointer<QtWebEngineCore::JavaScriptDialogController> controller,
                                    QObject *parent = nullptr);
    QWeakPointer<QtWebEngineCore::JavaScriptDialogController> m_controller;
    QString m_message;
    QString m_defaultPrompt;
    QString m_title;
    DialogType m_type;
    QUrl m_securityOrigin;
    bool m_accepted;
    friend class QQuickWebEngineViewPrivate;
    Q_DISABLE_COPY(QQuickWebEngineJavaScriptDialogRequest)
};

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineColorDialogRequest : public QObject {
    Q_OBJECT
public:

    Q_PROPERTY(QColor color READ color CONSTANT FINAL)
    Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted FINAL)
    QML_NAMED_ELEMENT(ColorDialogRequest)
    QML_ADDED_IN_VERSION(1, 4)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")

    ~QQuickWebEngineColorDialogRequest();

    QColor color() const;
    bool isAccepted() const;
    void setAccepted(bool accepted);

public slots:
    void dialogAccept(const QColor &color);
    void dialogReject();

private:
    QQuickWebEngineColorDialogRequest(QSharedPointer<QtWebEngineCore::ColorChooserController> controller,
                                    QObject *parent = nullptr);
    QWeakPointer<QtWebEngineCore::ColorChooserController> m_controller;
    QColor m_color;
    bool m_accepted;
    friend class QQuickWebEngineViewPrivate;
    Q_DISABLE_COPY(QQuickWebEngineColorDialogRequest)
};

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineFileDialogRequest : public QObject {
    Q_OBJECT
public:

    enum FileMode {
        FileModeOpen,
        FileModeOpenMultiple,
        FileModeUploadFolder,
        FileModeSave
    };
    Q_ENUM(FileMode)

    Q_PROPERTY(QString defaultFileName READ defaultFileName CONSTANT FINAL)
    Q_PROPERTY(QStringList acceptedMimeTypes READ acceptedMimeTypes CONSTANT FINAL)
    Q_PROPERTY(FileMode mode READ mode CONSTANT FINAL)
    Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted FINAL)
    QML_NAMED_ELEMENT(FileDialogRequest)
    QML_ADDED_IN_VERSION(1, 4)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")

    ~QQuickWebEngineFileDialogRequest();

    QStringList acceptedMimeTypes() const;
    QString defaultFileName() const;
    FileMode mode() const;
    bool isAccepted() const;
    void setAccepted(bool accepted);

public slots:
    void dialogAccept(const QStringList &files);
    void dialogReject();

private:
    QQuickWebEngineFileDialogRequest(QSharedPointer<QtWebEngineCore::FilePickerController> controller,
                                    QObject *parent = nullptr);
    QWeakPointer<QtWebEngineCore::FilePickerController> m_controller;
    QString m_filename;
    QStringList m_acceptedMimeTypes;
    FileMode m_mode;
    bool m_accepted;
    friend class QQuickWebEngineViewPrivate;
    Q_DISABLE_COPY(QQuickWebEngineFileDialogRequest)
};

class Q_WEBENGINEQUICK_PRIVATE_EXPORT QQuickWebEngineTooltipRequest : public QObject {
    Q_OBJECT
public:
    enum RequestType {
        Show,
        Hide,
    };
    Q_ENUM(RequestType)
    Q_PROPERTY(int x READ x CONSTANT FINAL)
    Q_PROPERTY(int y READ y CONSTANT FINAL)
    Q_PROPERTY(QString text READ text CONSTANT FINAL)
    Q_PROPERTY(RequestType type READ type CONSTANT FINAL)
    Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted FINAL)
    QML_NAMED_ELEMENT(TooltipRequest)
    QML_ADDED_IN_VERSION(1, 10)
    QML_EXTRA_VERSION(2, 0)
    QML_UNCREATABLE("")

    ~QQuickWebEngineTooltipRequest();
    int x() const;
    int y() const;
    QString text() const;
    RequestType type() const;
    bool isAccepted() const;
    void setAccepted(bool accepted);

private:
    QQuickWebEngineTooltipRequest(const QString &text = QString(),
                                QObject *parent = nullptr);
    QPoint m_position;
    QString m_text;
    RequestType m_type;
    bool m_accepted;
    friend class QQuickWebEngineViewPrivate;
    Q_DISABLE_COPY(QQuickWebEngineTooltipRequest)
};

QT_END_NAMESPACE

#endif // QQUICKWEBENGINDIALOGREQUESTS_P_H
