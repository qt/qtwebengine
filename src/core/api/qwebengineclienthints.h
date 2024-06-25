// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINECLIENTHINTS_H
#define QWEBENGINECLIENTHINTS_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>
#include <QtCore/qvariantmap.h>
#include <QtQml/qqmlregistration.h>

namespace QtWebEngineCore {
class ProfileAdapter;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineClientHints : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString arch READ arch WRITE setArch FINAL)
    Q_PROPERTY(QString platform READ platform WRITE setPlatform FINAL)
    Q_PROPERTY(QString model READ model WRITE setModel FINAL)
    Q_PROPERTY(bool mobile READ isMobile WRITE setIsMobile FINAL)
    Q_PROPERTY(QString fullVersion READ fullVersion WRITE setFullVersion FINAL)
    Q_PROPERTY(QString platformVersion READ platformVersion WRITE setPlatformVersion FINAL)
    Q_PROPERTY(QString bitness READ bitness WRITE setBitness FINAL)
    Q_PROPERTY(QVariantMap fullVersionList READ fullVersionList WRITE setFullVersionList FINAL)
    Q_PROPERTY(bool wow64 READ isWow64 WRITE setIsWow64 FINAL)

    Q_PROPERTY(bool isAllClientHintsEnabled READ isAllClientHintsEnabled WRITE setAllClientHintsEnabled FINAL)

public:
    QML_NAMED_ELEMENT(WebEngineClientHints)
    QML_UNCREATABLE("")
    QML_ADDED_IN_VERSION(6, 8)

    ~QWebEngineClientHints();

    QString arch() const;
    QString platform() const;
    QString model() const;
    bool isMobile() const;
    QString fullVersion() const;
    QString platformVersion() const;
    QString bitness() const;
    QVariantMap fullVersionList() const;
    bool isWow64() const;

    void setArch(const QString &);
    void setPlatform(const QString &);
    void setModel(const QString &);
    void setIsMobile(bool);
    void setFullVersion(const QString &);
    void setPlatformVersion(const QString &);
    void setBitness(const QString &);
    void setFullVersionList(const QVariantMap &);
    void setIsWow64(bool);

    bool isAllClientHintsEnabled();
    void setAllClientHintsEnabled(bool enabled);

    Q_INVOKABLE void resetAll();

private:
    explicit QWebEngineClientHints(QtWebEngineCore::ProfileAdapter *profileAdapter);

    Q_DISABLE_COPY(QWebEngineClientHints)
    friend class QWebEngineProfilePrivate;
    friend class QQuickWebEngineProfilePrivate;

    QPointer<QtWebEngineCore::ProfileAdapter> m_profileAdapter;
};

QT_END_NAMESPACE

#endif // QWEBENGINECLIENTHINTS_H
