/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "location_provider_qt.h"

#include <math.h>

#include "type_conversion.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <QtPositioning/QGeoPositionInfoSource>

#include "base/message_loop/message_loop.h"
#include "base/bind.h"
#include "content/browser/geolocation/geolocation_provider_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/geolocation_provider.h"

namespace QtWebEngineCore {

using content::BrowserThread;

class QtPositioningHelper : public QObject {
    Q_OBJECT
public:
    QtPositioningHelper(LocationProviderQt *provider);
    ~QtPositioningHelper();

    bool start(bool highAccuracy);
    void stop();
    void refresh();

private Q_SLOTS:
    void updatePosition(const QGeoPositionInfo &);
    void error(QGeoPositionInfoSource::Error positioningError);
    void timeout();

private:
    LocationProviderQt *m_locationProvider;
    QGeoPositionInfoSource *m_positionInfoSource;

    void postToLocationProvider(const base::Closure &task);
};

QtPositioningHelper::QtPositioningHelper(LocationProviderQt *provider)
    : m_locationProvider(provider)
    , m_positionInfoSource(0)
{
    Q_ASSERT(provider);
}

QtPositioningHelper::~QtPositioningHelper()
{
    m_locationProvider->m_positioningHelper = 0;
}

bool QtPositioningHelper::start(bool highAccuracy)
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    Q_UNUSED(highAccuracy);
    // FIXME: go through availableSources until one supports QGeoPositionInfoSource::SatellitePositioningMethods
    // for the highAccuracy case.
    m_positionInfoSource = QGeoPositionInfoSource::createDefaultSource(this);
    if (!m_positionInfoSource)
        return false;

    connect(m_positionInfoSource, &QGeoPositionInfoSource::positionUpdated, this, &QtPositioningHelper::updatePosition);
    // disambiguate the error getter and the signal in QGeoPositionInfoSource.
    connect(m_positionInfoSource, static_cast<void (QGeoPositionInfoSource::*)(QGeoPositionInfoSource::Error)>(&QGeoPositionInfoSource::error)
            , this, &QtPositioningHelper::error);
    connect(m_positionInfoSource, &QGeoPositionInfoSource::updateTimeout, this, &QtPositioningHelper::timeout);

    m_positionInfoSource->startUpdates();
    return true;
}

void QtPositioningHelper::stop()
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    if (!m_positionInfoSource)
        return;
    m_positionInfoSource->stopUpdates();
}

void QtPositioningHelper::refresh()
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    if (!m_positionInfoSource)
        return;
    m_positionInfoSource->stopUpdates();
}

void QtPositioningHelper::updatePosition(const QGeoPositionInfo &pos)
{
    if (!pos.isValid())
        return;
    Q_ASSERT(m_positionInfoSource->error() == QGeoPositionInfoSource::NoError);
    content::Geoposition newPos;
    newPos.error_code = content::Geoposition::ERROR_CODE_NONE;
    newPos.error_message.clear();

    newPos.timestamp = toTime(pos.timestamp());
    newPos.latitude = pos.coordinate().latitude();
    newPos.longitude = pos.coordinate().longitude();

    const double altitude = pos.coordinate().altitude();
    if (!qIsNaN(altitude))
        newPos.altitude = altitude;

    // Chromium's geoposition needs a valid (as in >=0.) accuracy field.
    // try and get an accuracy estimate from QGeoPositionInfo.
    // If we don't have any accuracy info, 100m seems a pesimistic enough default.
    if (!pos.hasAttribute(QGeoPositionInfo::VerticalAccuracy) && !pos.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
        newPos.accuracy = 100;
    else {
        const double vAccuracy = pos.hasAttribute(QGeoPositionInfo::VerticalAccuracy) ? pos.attribute(QGeoPositionInfo::VerticalAccuracy) : 0;
        const double hAccuracy = pos.hasAttribute(QGeoPositionInfo::HorizontalAccuracy) ? pos.attribute(QGeoPositionInfo::HorizontalAccuracy) : 0;
        newPos.accuracy = sqrt(vAccuracy * vAccuracy + hAccuracy * hAccuracy);
    }

    // And now the "nice to have" fields (-1 means invalid).
    newPos.speed =  pos.hasAttribute(QGeoPositionInfo::GroundSpeed) ? pos.attribute(QGeoPositionInfo::GroundSpeed) : -1;
    newPos.heading =  pos.hasAttribute(QGeoPositionInfo::Direction) ? pos.attribute(QGeoPositionInfo::Direction) : -1;

    postToLocationProvider(base::Bind(&LocationProviderQt::updatePosition, base::Unretained(m_locationProvider), newPos));
}

void QtPositioningHelper::error(QGeoPositionInfoSource::Error positioningError)
{
    Q_ASSERT(positioningError != QGeoPositionInfoSource::NoError);
    content::Geoposition newPos;
    switch (positioningError) {
    case QGeoPositionInfoSource::AccessError:
        newPos.error_code = content::Geoposition::ERROR_CODE_PERMISSION_DENIED;
        break;
    case QGeoPositionInfoSource::ClosedError:
    case QGeoPositionInfoSource::UnknownSourceError: // position unavailable is as good as it gets in Geoposition
    default:
        newPos.error_code = content::Geoposition::ERROR_CODE_POSITION_UNAVAILABLE;
        break;
    }
    postToLocationProvider(base::Bind(&LocationProviderQt::updatePosition, base::Unretained(m_locationProvider), newPos));
}

void QtPositioningHelper::timeout()
{
    content::Geoposition newPos;
    // content::Geoposition::ERROR_CODE_TIMEOUT is not handled properly in the renderer process, and the timeout
    // argument used in JS never comes all the way to the browser process.
    // Let's just treat it like any other error where the position is unavailable.
    newPos.error_code = content::Geoposition::ERROR_CODE_POSITION_UNAVAILABLE;
    postToLocationProvider(base::Bind(&LocationProviderQt::updatePosition, base::Unretained(m_locationProvider), newPos));
}

inline void QtPositioningHelper::postToLocationProvider(const base::Closure &task)
{
    static_cast<content::GeolocationProviderImpl*>(content::GeolocationProvider::GetInstance())->message_loop()->PostTask(FROM_HERE, task);
}

LocationProviderQt::LocationProviderQt()
    : m_positioningHelper(0)
{
}

LocationProviderQt::~LocationProviderQt()
{
    m_positioningHelper->deleteLater();
}

bool LocationProviderQt::StartProvider(bool highAccuracy)
{
    QThread *guiThread = qApp->thread();
    if (!m_positioningHelper) {
        m_positioningHelper = new QtPositioningHelper(this);
        m_positioningHelper->moveToThread(guiThread);
    }
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, base::Bind(base::IgnoreResult(&QtPositioningHelper::start)
                                                                 , base::Unretained(m_positioningHelper), highAccuracy));
    return true;
}

void LocationProviderQt::StopProvider()
{
    if (m_positioningHelper)
        BrowserThread::PostTask(BrowserThread::UI,FROM_HERE, base::Bind(&QtPositioningHelper::stop
                                                                     , base::Unretained(m_positioningHelper)));
}

void LocationProviderQt::RequestRefresh()
{
    if (m_positioningHelper)
        BrowserThread::PostTask(BrowserThread::UI,FROM_HERE, base::Bind(&QtPositioningHelper::refresh
                                                                     , base::Unretained(m_positioningHelper)));
}

void LocationProviderQt::OnPermissionGranted()
{
    RequestRefresh();
}

void LocationProviderQt::updatePosition(const content::Geoposition &position)
{
    m_lastKnownPosition = position;
    NotifyCallback(position);
}

} // namespace QtWebEngineCore

#include "location_provider_qt.moc"
