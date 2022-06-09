// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINESCRIPT_H
#define QWEBENGINESCRIPT_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtCore/qurl.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>

namespace QtWebEngineCore {
class UserScript;
} // namespace

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineScript
{

    Q_GADGET
    Q_PROPERTY(QString name READ name WRITE setName FINAL)
    Q_PROPERTY(QUrl sourceUrl READ sourceUrl WRITE setSourceUrl FINAL)
    Q_PROPERTY(QString sourceCode READ sourceCode WRITE setSourceCode FINAL)
    Q_PROPERTY(InjectionPoint injectionPoint READ injectionPoint WRITE setInjectionPoint FINAL)
    Q_PROPERTY(quint32 worldId READ worldId WRITE setWorldId FINAL)
    Q_PROPERTY(bool runsOnSubFrames READ runsOnSubFrames WRITE setRunsOnSubFrames FINAL)

public:

    enum InjectionPoint {
        Deferred,
        DocumentReady,
        DocumentCreation
    };

    Q_ENUM(InjectionPoint)

    enum ScriptWorldId {
        MainWorld = 0,
        ApplicationWorld,
        UserWorld
    };

    Q_ENUM(ScriptWorldId)

    QWebEngineScript();
    QWebEngineScript(const QWebEngineScript &other);
    ~QWebEngineScript();

    QWebEngineScript &operator=(const QWebEngineScript &other);

    QString name() const;
    void setName(const QString &);

    QUrl sourceUrl() const;
    void setSourceUrl(const QUrl &url);

    QString sourceCode() const;
    void setSourceCode(const QString &);

    InjectionPoint injectionPoint() const;
    void setInjectionPoint(InjectionPoint);

    quint32 worldId() const;
    void setWorldId(quint32);

    bool runsOnSubFrames() const;
    void setRunsOnSubFrames(bool on);

    bool operator==(const QWebEngineScript &other) const;
    inline bool operator!=(const QWebEngineScript &other) const
    { return !operator==(other); }
    void swap(QWebEngineScript &other) noexcept { d.swap(other.d); }

private:
    friend class QWebEngineScriptCollectionPrivate;
    QWebEngineScript(const QtWebEngineCore::UserScript &);

    QSharedDataPointer<QtWebEngineCore::UserScript> d;
};

Q_DECLARE_TYPEINFO(QWebEngineScript, Q_RELOCATABLE_TYPE);

#ifndef QT_NO_DEBUG_STREAM
Q_WEBENGINECORE_EXPORT QDebug operator<<(QDebug, const QWebEngineScript &);
#endif

QT_END_NAMESPACE

#endif // QWEBENGINESCRIPT_H
