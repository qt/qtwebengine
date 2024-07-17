// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "desktop_media_controller.h"
#include "qwebenginedesktopmediarequest.h"
#include "qwebenginedesktopmediarequest_p.h"

QT_BEGIN_NAMESPACE

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QWebEngineDesktopMediaRequestPrivate)

/*!
    \class QWebEngineDesktopMediaRequest
    \brief A request for populating a dialog with available sources for screen capturing.

    \since 6.7

    \inmodule QtWebEngineCore

    To allow web applications to capture contents of a display, applications must connect
    to QWebEnginePage::desktopMediaRequested, which takes a QWebEngineDesktopMediaRequest
    instance as an argument.

    If a web application requests access to the contents of a display,
    QWebEnginePage::desktopMediaRequested will be emitted with a
    QWebEngineDesktopMediaRequest instance as an argument which holds references to
    QAbstractListModels for available windows and screens that can be captured.

    The data model's \e Qt::DisplayRole specifies the name of the source which is the title of a
    window or the number of the display.
    The model is dynamically updates if the available list of sources has changed e.g a window is
    opened/closed.

    The signal handler needs to then either call QWebEngineDesktopMediaRequest:selectScreen() or
    QWebEngineDesktopMediaRequest::selectWindow() to accept the request and start screensharing.
    \sa QWebEnginePage::desktopMediaRequested().
*/

class QWebEngineMediaSourceModel : public QAbstractListModel
{
public:
    ~QWebEngineMediaSourceModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    friend class QWebEngineDesktopMediaRequestPrivate;
    explicit QWebEngineMediaSourceModel(QtWebEngineCore::DesktopMediaListQt *mediaList);
    QtWebEngineCore::DesktopMediaListQt *m_mediaList;
};

QWebEngineMediaSourceModel::QWebEngineMediaSourceModel(QtWebEngineCore::DesktopMediaListQt *mediaList)
    : m_mediaList(mediaList)
{
    QObject::connect(m_mediaList, &QtWebEngineCore::DesktopMediaListQt::sourceAdded, this,
                     [this](int index) {
                        beginInsertRows(QModelIndex(), index, index);
                        endInsertRows();
                     });
    QObject::connect(m_mediaList, &QtWebEngineCore::DesktopMediaListQt::sourceRemoved, this,
                     [this](int index) {
                         beginRemoveRows(QModelIndex(), index, index);
                         endRemoveRows();
                     });
    QObject::connect(m_mediaList, &QtWebEngineCore::DesktopMediaListQt::sourceMoved, this,
                     [this](int oldIndex, int newIndex) {
                         beginMoveRows(QModelIndex(), oldIndex, oldIndex, QModelIndex(), newIndex);
                         endMoveRows();
                     });
    QObject::connect(m_mediaList, &QtWebEngineCore::DesktopMediaListQt::sourceNameChanged, this,
                     [this](int index) {
                         Q_EMIT dataChanged(QModelIndex(), QModelIndex(),
                                            { Qt::DisplayRole });
                     });
}

QWebEngineMediaSourceModel::~QWebEngineMediaSourceModel() { }

int QWebEngineMediaSourceModel::rowCount(const QModelIndex &) const
{
    return m_mediaList->getSourceCount();
}

QVariant QWebEngineMediaSourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return m_mediaList->getSourceName(index.row());
    default:
        break;
    }
    return QVariant();
}

QWebEngineDesktopMediaRequestPrivate::QWebEngineDesktopMediaRequestPrivate(
        QtWebEngineCore::DesktopMediaController *controller)
    : controller(controller)
    , m_screensModel(new QWebEngineMediaSourceModel(controller->screens()))
    , m_windowsModel(new QWebEngineMediaSourceModel(controller->windows()))
{
}

QWebEngineDesktopMediaRequestPrivate::~QWebEngineDesktopMediaRequestPrivate()
{
    // Keep old behavior, if there were no user action select the primary screen.
    if (!didSelectOrCancel)
        controller->screens()->getSourceCount() > 0 ? controller->selectScreen(0)
                                                    : controller->cancel();
}

void QWebEngineDesktopMediaRequestPrivate::selectWindow(const QModelIndex &index)
{
    Q_ASSERT(index.model() == m_windowsModel.get());
    if (!index.isValid())
        return;
    didSelectOrCancel = true;
    controller->selectWindow(index.row());
}

void QWebEngineDesktopMediaRequestPrivate::selectScreen(const QModelIndex &index)
{
    Q_ASSERT(index.model() == m_screensModel.get());
    if (!index.isValid())
        return;
    didSelectOrCancel = true;
    controller->selectScreen(index.row());
}

void QWebEngineDesktopMediaRequestPrivate::cancel()
{
    // Notifies webrtc so it can free up it's resources.
    didSelectOrCancel = true;
    controller->cancel();
}

QWebEngineDesktopMediaRequest::QWebEngineDesktopMediaRequest(
        QtWebEngineCore::DesktopMediaController *controller)
    : d(new QWebEngineDesktopMediaRequestPrivate(controller))
{
}

QWebEngineDesktopMediaRequest::~QWebEngineDesktopMediaRequest() = default;

QWebEngineDesktopMediaRequest::QWebEngineDesktopMediaRequest(
        const QWebEngineDesktopMediaRequest &other) noexcept = default;

QWebEngineDesktopMediaRequest &QWebEngineDesktopMediaRequest::operator=(
        const QWebEngineDesktopMediaRequest &other) noexcept = default;

QWebEngineDesktopMediaRequest::QWebEngineDesktopMediaRequest(
        QWebEngineDesktopMediaRequest &&other) noexcept = default;

/*!
    Returns a QAbstractListModel for the available screens.

    \sa windowsModel()
*/
QAbstractListModel *QWebEngineDesktopMediaRequest::screensModel() const
{
    return d->m_screensModel.get();
}

/*!
    Returns a QAbstractListModel for the available windows.

    \sa screensModel()
*/
QAbstractListModel *QWebEngineDesktopMediaRequest::windowsModel() const
{
    return d->m_windowsModel.get();
}

/*!
    Selects the window on the \a index to be captured.

    \sa QWebEngineDesktopMediaRequest::selectScreen()
*/
void QWebEngineDesktopMediaRequest::selectWindow(const QModelIndex &index) const
{
    d->selectWindow(index);
}

/*!
    Selects the screen on the \a index to be captured.

    \sa QWebEngineDesktopMediaRequest::selectWindow()
*/
void QWebEngineDesktopMediaRequest::selectScreen(const QModelIndex &index) const
{
    d->selectScreen(index);
}

/*!
    Rejects a request. Screen capturing will be aborted.
*/
void QWebEngineDesktopMediaRequest::cancel() const
{
    d->cancel();
}

QT_END_NAMESPACE

#include "moc_qwebenginedesktopmediarequest.cpp"
