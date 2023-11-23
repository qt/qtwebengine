// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef RESOURCEREQUESTBODY_QT_H
#define RESOURCEREQUESTBODY_QT_H

#include <QtWebEngineCore/private/qtwebenginecoreglobal_p.h>
#include <QtCore/QIODevice>
#include <QtCore/QFile>
#include <QtCore/QUrl>

namespace network {
class ResourceRequestBody;
namespace mojom {
class DataPipeGetter;
class ChunkedDataPipeGetter;
}
}

namespace base {
class FilePath;
}

namespace mojo {
template<typename T>
class Remote;
template<typename T>
class ScopedHandleBase;
class DataPipeConsumerHandle;
}

namespace QtWebEngineCore {

class Q_WEBENGINECORE_EXPORT ResourceRequestBody : public QIODevice
{
    Q_OBJECT
public:
    explicit ResourceRequestBody(network::ResourceRequestBody *requestBody,
                                 QObject *parent = nullptr);
    ~ResourceRequestBody();

    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
    bool isSequential() const override;

    void appendFilesForTest(const QString &path);

private:
    network::ResourceRequestBody *const m_requestBody;

    std::size_t m_dataElementsIdx;
    std::size_t m_dataElementBytesIdx;
    std::size_t m_dataElementFileIdx;

    void readDataElementBytes(const std::vector<uint8_t> &dataElement, qint64 &bytesRead,
                              const qint64 &maxSize, char **data);
    void readDataElementFile(const base::FilePath &filePath, const qint64 &offset,
                             const qint64 &length, qint64 &bytesRead, const qint64 &maxSize,
                             char **data);
    mojo::ScopedHandleBase<mojo::DataPipeConsumerHandle>
    getConsumerHandleFromPipeGetter(mojo::Remote<network::mojom::DataPipeGetter> &pipeGetter);
    void
    readDataElementPipe(const mojo::ScopedHandleBase<mojo::DataPipeConsumerHandle> &consumerHandle,
                        qint64 &bytesRead, const qint64 &maxSize, char **data);
    void pipeGetterOnReadComplete(int32_t status, uint64_t size);
};

} // namespace QtWebEngineCore

#endif // RESOURCEREQUESTBODY_QT_H
