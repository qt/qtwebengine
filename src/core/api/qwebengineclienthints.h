// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINECLIENTHINTS_H
#define QWEBENGINECLIENTHINTS_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qobject.h>
#include <QtCore/qhash.h>

namespace QtWebEngineCore {
class ProfileAdapter;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineClientHints
{
    Q_GADGET
    Q_PROPERTY(QString arch READ arch WRITE setArch)
    Q_PROPERTY(QString platform READ platform WRITE setPlatform)
    Q_PROPERTY(QString model READ model WRITE setModel)
    Q_PROPERTY(bool mobile READ isMobile WRITE setIsMobile)
    Q_PROPERTY(QString fullVersion READ fullVersion WRITE setFullVersion)
    Q_PROPERTY(QString platformVersion READ platformVersion WRITE setPlatformVersion)
    Q_PROPERTY(QString bitness READ bitness WRITE setBitness)
    Q_PROPERTY(QHash<QString,QString> fullVersionList READ fullVersionList WRITE setFullVersionList)
    Q_PROPERTY(bool wow64 READ isWow64 WRITE setIsWow64)

    Q_PROPERTY(bool isAllClientHintsEnabled READ isAllClientHintsEnabled WRITE setAllClientHintsEnabled)

public:
    ~QWebEngineClientHints();

    QString arch() const;
    QString platform() const;
    QString model() const;
    bool isMobile() const;
    QString fullVersion() const;
    QString platformVersion() const;
    QString bitness() const;
    QHash<QString,QString> fullVersionList() const;
    bool isWow64() const;

    void setArch(const QString &);
    void setPlatform(const QString &);
    void setModel(const QString &);
    void setIsMobile(const bool);
    void setFullVersion(const QString &);
    void setPlatformVersion(const QString &);
    void setBitness(const QString &);
    void setFullVersionList(const QHash<QString,QString> &);
    void setIsWow64(const bool);

    bool isAllClientHintsEnabled();
    void setAllClientHintsEnabled(bool enabled);

    void resetAll();

private:
    explicit QWebEngineClientHints(QtWebEngineCore::ProfileAdapter *profileAdapter);
    Q_DISABLE_COPY(QWebEngineClientHints)
    friend class QWebEngineProfilePrivate;
    friend class QQuickWebEngineProfilePrivate;

    QtWebEngineCore::ProfileAdapter *m_profileAdapter;
};

QT_END_NAMESPACE

#endif // QWEBENGINECLIENTHINTS_H
