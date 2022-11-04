// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBENGINEDESKTOPMEDIAREQUEST_H
#define QWEBENGINEDESKTOPMEDIAREQUEST_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qobject.h>
#include <QtWebEngineCore/qtwebenginecoreglobal.h>

namespace QtWebEngineCore {
class DesktopMediaController;
}

QT_BEGIN_NAMESPACE
class QWebEnginePagePrivate;
class QQuickWebEngineViewPrivate;
class QWebEngineMediaSourceModelPrivate;
class QWebEngineDesktopMediaRequestPrivate;

class Q_WEBENGINECORE_EXPORT QWebEngineMediaSourceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles { NameRole = Qt::UserRole };
    ~QWebEngineMediaSourceModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    friend class QWebEngineDesktopMediaRequestPrivate;
    explicit QWebEngineMediaSourceModel(QWebEngineMediaSourceModelPrivate *dd);
    std::unique_ptr<QWebEngineMediaSourceModelPrivate> d;
};

class QWebEngineDesktopMediaRequest
{
    Q_GADGET_EXPORT(Q_WEBENGINECORE_EXPORT)
    Q_PROPERTY(QWebEngineMediaSourceModel *screensModel READ screensModel FINAL)
    Q_PROPERTY(QWebEngineMediaSourceModel *windowsModel READ windowsModel FINAL)

public:
    Q_WEBENGINECORE_EXPORT
    QWebEngineDesktopMediaRequest(const QWebEngineDesktopMediaRequest &other) noexcept = default;
    Q_WEBENGINECORE_EXPORT
    QWebEngineDesktopMediaRequest(QWebEngineDesktopMediaRequest &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QWebEngineDesktopMediaRequest)
    Q_WEBENGINECORE_EXPORT ~QWebEngineDesktopMediaRequest() = default;
    void swap(QWebEngineDesktopMediaRequest &other) noexcept { d.swap(other.d); }

    Q_WEBENGINECORE_EXPORT QWebEngineMediaSourceModel *screensModel() const;
    Q_WEBENGINECORE_EXPORT QWebEngineMediaSourceModel *windowsModel() const;

    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void selectScreen(const QModelIndex &index) const;
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void selectWindow(const QModelIndex &index) const;
    Q_WEBENGINECORE_EXPORT Q_INVOKABLE void cancel() const;

private:
    friend class QWebEnginePagePrivate;
    friend class QQuickWebEngineViewPrivate;
    Q_WEBENGINECORE_EXPORT explicit QWebEngineDesktopMediaRequest(
            QtWebEngineCore::DesktopMediaController *controller);
    QSharedPointer<QWebEngineDesktopMediaRequestPrivate> d;
};
Q_DECLARE_SHARED(QWebEngineDesktopMediaRequest)

QT_END_NAMESPACE

#endif // QWEBENGINEDESKTOPMEDIAREQUEST_H
