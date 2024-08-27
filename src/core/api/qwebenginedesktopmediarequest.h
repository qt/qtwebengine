// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEDESKTOPMEDIAREQUEST_H
#define QWEBENGINEDESKTOPMEDIAREQUEST_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qobject.h>
#include <QtWebEngineCore/qtwebenginecoreglobal.h>

namespace QtWebEngineCore {
class DesktopMediaController;
}

QT_BEGIN_NAMESPACE
class QWebEnginePagePrivate;
class QQuickWebEngineViewPrivate;
class QWebEngineDesktopMediaRequestPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QWebEngineDesktopMediaRequestPrivate,
                                                 Q_WEBENGINECORE_EXPORT)

class QWebEngineDesktopMediaRequest
{
    Q_GADGET_EXPORT(Q_WEBENGINECORE_EXPORT)
    Q_PROPERTY(QAbstractListModel *screensModel READ screensModel FINAL)
    Q_PROPERTY(QAbstractListModel *windowsModel READ windowsModel FINAL)

public:
    Q_WEBENGINECORE_EXPORT ~QWebEngineDesktopMediaRequest();

    Q_WEBENGINECORE_EXPORT
    QWebEngineDesktopMediaRequest(const QWebEngineDesktopMediaRequest &other) noexcept;
    Q_WEBENGINECORE_EXPORT
    QWebEngineDesktopMediaRequest(QWebEngineDesktopMediaRequest &&other) noexcept;
    Q_WEBENGINECORE_EXPORT
    QWebEngineDesktopMediaRequest &operator=(const QWebEngineDesktopMediaRequest &other) noexcept;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QWebEngineDesktopMediaRequest)
    void swap(QWebEngineDesktopMediaRequest &other) noexcept { d.swap(other.d); }

    Q_WEBENGINECORE_EXPORT QAbstractListModel *screensModel() const;
    Q_WEBENGINECORE_EXPORT QAbstractListModel *windowsModel() const;

    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void selectScreen(const QModelIndex &index) const;
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void selectWindow(const QModelIndex &index) const;
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void cancel() const;

private:
    friend class QWebEnginePagePrivate;
    friend class QQuickWebEngineViewPrivate;
    Q_WEBENGINECORE_EXPORT explicit QWebEngineDesktopMediaRequest(
            QtWebEngineCore::DesktopMediaController *controller);
    QExplicitlySharedDataPointer<QWebEngineDesktopMediaRequestPrivate> d;
};
Q_DECLARE_SHARED(QWebEngineDesktopMediaRequest)

QT_END_NAMESPACE

#endif // QWEBENGINEDESKTOPMEDIAREQUEST_H
