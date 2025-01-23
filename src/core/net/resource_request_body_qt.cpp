// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "resource_request_body_qt.h"
#include "type_conversion.h"

#include "services/network/public/cpp/resource_request_body.h"
#include "services/network/public/mojom/data_pipe_getter.mojom.h"
#include "services/network/public/mojom/url_request.mojom-shared.h"
#include "mojo/public/cpp/bindings/remote.h"

using namespace Qt::StringLiterals;

namespace QtWebEngineCore {

ResourceRequestBody::ResourceRequestBody(network::ResourceRequestBody *requestBody, QObject *parent)
    : QIODevice(parent)
    , m_requestBody(requestBody)
    , m_dataElementsIdx(0)
    , m_dataElementBytesIdx(0)
    , m_dataElementFileIdx(0)
{};

ResourceRequestBody::~ResourceRequestBody(){};

qint64 ResourceRequestBody::readData(char *data, qint64 maxSize)
{
    if (!m_requestBody)
        return -1;

    const std::size_t dataElementsSize = m_requestBody->elements()->size();
    if (m_dataElementsIdx == dataElementsSize)
        return -1;

    qint64 bytesRead = 0;
    const std::vector<network::DataElement> *elements = m_requestBody->elements();
    while (bytesRead < maxSize && m_dataElementsIdx < dataElementsSize) {
        const network::DataElement &currentDataElement = elements->at(m_dataElementsIdx);

        switch (currentDataElement.type()) {
            case network::mojom::DataElementDataView::Tag::kBytes: {
                readDataElementBytes(currentDataElement.As<network::DataElementBytes>().bytes(),
                                     bytesRead, maxSize, &data);
                break;
            }
            case network::mojom::DataElementDataView::Tag::kFile: {
                const network::DataElementFile file = currentDataElement.As<network::DataElementFile>();
                const qint64 offset = file.offset();
                const qint64 length = file.length();
                readDataElementFile(file.path(), offset, length, bytesRead, maxSize, &data);
                break;
            }
            case network::mojom::DataElementDataView::Tag::kDataPipe: {
                mojo::Remote<network::mojom::DataPipeGetter> pipeGetter;
                pipeGetter.Bind(
                        currentDataElement.As<network::DataElementDataPipe>().CloneDataPipeGetter());
                const mojo::ScopedHandleBase<mojo::DataPipeConsumerHandle> consumerHandle =
                        getConsumerHandleFromPipeGetter(pipeGetter);
                readDataElementPipe(consumerHandle, bytesRead, maxSize, &data);
                break;
            }
            case network::mojom::DataElementDataView::Tag::kChunkedDataPipe: {
                setErrorString(u"Chunked data pipe is used in request body upload, which "
                               "is currently not supported"_s);
                // Nothing should come before or after DataElementChunkedDataPipe
                return -1;
            }
        }

        if (bytesRead == maxSize || m_dataElementsIdx == dataElementsSize)
            break;
    }

    return bytesRead;
}

// We don't want to write, ever
qint64 ResourceRequestBody::writeData(const char *data, qint64 maxSize)
{
    return -1;
}

bool ResourceRequestBody::isSequential() const
{
    return true;
}

void ResourceRequestBody::readDataElementBytes(const std::vector<uint8_t> &dataElement,
                                               qint64 &bytesRead, const qint64 &maxSize,
                                               char **data)
{
    const std::size_t dataElementSize = dataElement.size();
    const std::size_t bytesToRead = std::min(dataElementSize, static_cast<std::size_t>(maxSize));

    std::memcpy(*data, dataElement.data(), bytesToRead);
    *data += bytesToRead;
    m_dataElementBytesIdx += bytesToRead;
    bytesRead += bytesToRead;

    if (m_dataElementBytesIdx == dataElementSize) {
        m_dataElementsIdx++;
        m_dataElementBytesIdx = 0;
    }
}

void ResourceRequestBody::readDataElementFile(const base::FilePath &filePath, const qint64 &offset,
                                              const qint64 &length, qint64 &bytesRead,
                                              const qint64 &maxSize, char **data)
{
    QFile file(toQt(filePath.value()));
    const qint64 realOffset = offset + m_dataElementFileIdx;
    const std::size_t fileSize = std::min(file.size(), length) - realOffset;
    const std::size_t bytesToRead = std::min(fileSize, static_cast<std::size_t>(maxSize));

    if (!file.open(QFile::ReadOnly)) {
        m_dataElementsIdx++;
        m_dataElementFileIdx = 0;
        setErrorString(u"Error while reading from file, skipping remaining content of "_s
                       % file.fileName() % u": "_s % file.errorString());
        return;
    }

    file.seek(realOffset);

    std::memcpy(*data, file.read(bytesToRead).data(), bytesToRead);
    *data += bytesToRead;
    m_dataElementFileIdx += bytesToRead;
    bytesRead += bytesToRead;

    file.close();

    if (m_dataElementFileIdx == fileSize) {
        m_dataElementsIdx++;
        m_dataElementFileIdx = 0;
    }
}

mojo::ScopedHandleBase<mojo::DataPipeConsumerHandle>
ResourceRequestBody::getConsumerHandleFromPipeGetter(
        mojo::Remote<network::mojom::DataPipeGetter> &pipeGetter)
{
    mojo::ScopedHandleBase<mojo::DataPipeProducerHandle> producerHandle;
    mojo::ScopedHandleBase<mojo::DataPipeConsumerHandle> consumerHandle;
    mojo::CreateDataPipe(nullptr, producerHandle, consumerHandle);
    base::WeakPtrFactory<ResourceRequestBody> weakPtrFactory{ this };
    pipeGetter->Read(std::move(producerHandle),
                     base::BindOnce(&ResourceRequestBody::pipeGetterOnReadComplete,
                                    weakPtrFactory.GetWeakPtr()));

    return consumerHandle;
}

void ResourceRequestBody::readDataElementPipe(
        const mojo::ScopedHandleBase<mojo::DataPipeConsumerHandle> &consumerHandle,
        qint64 &bytesRead, qint64 maxSize, char **data)
{
    MojoResult result;
    do {
        size_t bytesToRead = 1;
        base::span<uint8_t> buffer = base::make_span(reinterpret_cast<uint8_t*>(*data), reinterpret_cast<uint8_t*>(*data) + maxSize);
        result = consumerHandle->ReadData(MOJO_READ_DATA_FLAG_NONE, buffer, bytesToRead);

        if (result == MOJO_RESULT_OK) {
            *data += bytesToRead;
            bytesRead += bytesToRead;
            maxSize -= bytesToRead;
        } else if (result != MOJO_RESULT_SHOULD_WAIT && result != MOJO_RESULT_FAILED_PRECONDITION) {
            setErrorString("Error while reading from data pipe, skipping "
                           "remaining content of data pipe. Mojo error code: "_L1
                           + QString::number(result));
        }
    } while ((result == MOJO_RESULT_SHOULD_WAIT || result == MOJO_RESULT_OK)
             && maxSize > 0);

    m_dataElementsIdx++;
}

void ResourceRequestBody::pipeGetterOnReadComplete(int32_t status, uint64_t size) { }

} // namespace QtWebEngineCore
