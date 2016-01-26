/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtPositioning/qgeopositioninfosource.h>
#include <QtPositioning/qgeopositioninfosourcefactory.h>
#include <QObject>
#include <QtPlugin>

class DummySource : public QGeoPositionInfoSource
{
    Q_OBJECT

public:
    DummySource(QObject *parent=0);

    void startUpdates() {}
    void stopUpdates() {}
    void requestUpdate(int) {}

    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const;
    PositioningMethods supportedPositioningMethods() const;

    int minimumUpdateInterval() const;
    Error error() const;
};

DummySource::DummySource(QObject *parent) :
    QGeoPositionInfoSource(parent)
{
}

QGeoPositionInfoSource::Error DummySource::error() const
{
    return QGeoPositionInfoSource::NoError;
}

int DummySource::minimumUpdateInterval() const
{
    return 1000;
}

QGeoPositionInfo DummySource::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    Q_UNUSED(fromSatellitePositioningMethodsOnly);
    return QGeoPositionInfo(QGeoCoordinate(54.186824, 12.087262), QDateTime::currentDateTime());
}

QGeoPositionInfoSource::PositioningMethods DummySource::supportedPositioningMethods() const
{
    return QGeoPositionInfoSource::AllPositioningMethods;
}


class QGeoPositionInfoSourceFactoryTest : public QObject, public QGeoPositionInfoSourceFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.position.sourcefactory/5.0"
                      FILE "plugin.json")
    Q_INTERFACES(QGeoPositionInfoSourceFactory)

public:
    QGeoPositionInfoSource *positionInfoSource(QObject *parent);
    QGeoSatelliteInfoSource *satelliteInfoSource(QObject *parent);
    QGeoAreaMonitorSource *areaMonitor(QObject *parent);
};

QGeoPositionInfoSource *QGeoPositionInfoSourceFactoryTest::positionInfoSource(QObject *parent)
{
    return new DummySource(parent);
}

QGeoSatelliteInfoSource *QGeoPositionInfoSourceFactoryTest::satelliteInfoSource(QObject *)
{
    return 0;
}

QGeoAreaMonitorSource *QGeoPositionInfoSourceFactoryTest::areaMonitor(QObject* )
{
    return 0;
}

#include "plugin.moc"
