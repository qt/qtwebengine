// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#ifndef PRINTER_WORKER_H
#define PRINTER_WORKER_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>

#include <QtCore/qobject.h>
#include <QtCore/qsharedpointer.h>

QT_BEGIN_NAMESPACE
class QPagedPaintDevice;
QT_END_NAMESPACE

namespace QtWebEngineCore {

class Q_WEBENGINECORE_PRIVATE_EXPORT PrinterWorker : public QObject
{
    Q_OBJECT
public:
    PrinterWorker(QSharedPointer<QByteArray> data, QPagedPaintDevice *device);
    virtual ~PrinterWorker();

    int m_deviceResolution;
    bool m_firstPageFirst;
    int m_documentCopies;
    bool m_collateCopies;

public Q_SLOTS:
    void print();

Q_SIGNALS:
    void resultReady(bool success);

private:
    Q_DISABLE_COPY(PrinterWorker)

    QSharedPointer<QByteArray> m_data;
    QPagedPaintDevice *m_device;
};

} // namespace QtWebEngineCore

Q_DECLARE_METATYPE(QtWebEngineCore::PrinterWorker *)

#endif // PRINTER_WORKER_H
