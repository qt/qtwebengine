// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEFRAME_H
#define QWEBENGINEFRAME_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>
#include <QtQml/qqmlregistration.h>
#include <QtCore/qcompare.h>
#include <QtCore/QList>
#include <QtCore/QSizeF>
#include <QtCore/QString>
#include <QtCore/QUrl>

namespace QtWebEngineCore {
class WebContentsAdapterClient;
}

QT_BEGIN_NAMESPACE

class Q_WEBENGINECORE_EXPORT QWebEngineFrame
{
    Q_GADGET

    Q_PROPERTY(bool isValid READ isValid FINAL)
    Q_PROPERTY(QString name READ name FINAL)
    Q_PROPERTY(QString htmlName READ htmlName FINAL)
    Q_PROPERTY(QUrl url READ url FINAL)
    Q_PROPERTY(QSizeF size READ size FINAL)

public:
    QML_VALUE_TYPE(webEngineFrame)
    QML_ADDED_IN_VERSION(6, 8)

    bool isValid() const;
    QString name() const;
    QString htmlName() const;
    QList<QWebEngineFrame> children() const;
    QUrl url() const;
    QSizeF size() const;

    friend inline bool comparesEqual(const QWebEngineFrame &lhs,
                                     const QWebEngineFrame &rhs) noexcept
    {
        return lhs.m_adapterClient == rhs.m_adapterClient && lhs.m_id == rhs.m_id;
    }

    Q_DECLARE_EQUALITY_COMPARABLE(QWebEngineFrame);

private:
    friend class QWebEnginePage;
    friend class QQuickWebEngineView;

    QWebEngineFrame(QtWebEngineCore::WebContentsAdapterClient *page, quint64 id);

    QtWebEngineCore::WebContentsAdapterClient *m_adapterClient;
    quint64 m_id;
};

QT_END_NAMESPACE

#endif // QWEBENGINEFRAME_H
