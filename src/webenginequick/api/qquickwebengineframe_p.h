// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKWEBENGINEFRAME_H
#define QQUICKWEBENGINEFRAME_H

#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <QtWebEngineCore/qwebengineframe.h>
#include <QtQml/qqmlregistration.h>
#include <QtQml/qjsvalue.h>

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

QT_BEGIN_NAMESPACE

class QQuickWebEngineFrame : public QWebEngineFrame
{
    Q_GADGET_EXPORT(Q_WEBENGINEQUICK_EXPORT)
    Q_PROPERTY(QList<QQuickWebEngineFrame> children READ children CONSTANT REVISION(6, 10) FINAL)
    QML_VALUE_TYPE(webEngineFrame)
    QML_ADDED_IN_VERSION(6, 8)
    QML_FOREIGN(QWebEngineFrame)
    QML_EXTENDED(QQuickWebEngineFrame)

public:
    QQuickWebEngineFrame();
    Q_WEBENGINEQUICK_EXPORT Q_INVOKABLE void runJavaScript(const QString &script,
                                                           const QJSValue &callback);
    Q_WEBENGINEQUICK_EXPORT Q_INVOKABLE void runJavaScript(const QString &script, quint32 worldId,
                                                           const QJSValue &callback);
    Q_WEBENGINEQUICK_EXPORT Q_INVOKABLE void printToPdf(const QJSValue &callback);
    QList<QQuickWebEngineFrame> children() const;

private:
    friend class QQuickWebEngineView;
    friend class QQuickWebEngineViewPrivate;
    explicit QQuickWebEngineFrame(QWeakPointer<QtWebEngineCore::WebContentsAdapter> adapter,
                                  quint64 id);
};

// note base class has only defaulted destructor
static_assert(sizeof(QWebEngineFrame) == sizeof(QQuickWebEngineFrame));

QT_END_NAMESPACE

#endif // QQUICKWEBENGINEFRAME_H
