// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qpdfiohandler_p.h"

QT_BEGIN_NAMESPACE

class QPdfPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QImageIOHandlerFactoryInterface_iid FILE "pdf.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

QImageIOPlugin::Capabilities QPdfPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "pdf")
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return {};

    Capabilities cap;
    if (device->isReadable() && QPdfIOHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *QPdfPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QPdfIOHandler *hand = new QPdfIOHandler();
    hand->setDevice(device);
    hand->setFormat(format);
    return hand;
}

QT_END_NAMESPACE

#include "main.moc"
