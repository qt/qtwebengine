// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "native_skia_output_device.h"

#include "type_conversion.h"

#include "components/viz/common/resources/shared_image_format.h"
#include "components/viz/common/resources/shared_image_format_utils.h"
#include "components/viz/service/display_embedder/skia_output_surface_dependency.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/command_buffer/common/shared_image_usage.h"
#include "gpu/command_buffer/service/shared_image/shared_image_factory.h"
#include "gpu/command_buffer/service/shared_image/shared_image_representation.h"
#include "gpu/command_buffer/service/skia_utils.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "third_party/skia/include/core/SkSurfaceProps.h"
#include "ui/gfx/native_pixmap.h"
#include "ui/gfx/gpu_fence.h"
#include "ui/gl/gl_fence.h"

namespace QtWebEngineCore {

namespace {

// Helper function for moving a GpuFence from a fence handle to a unique_ptr.
std::unique_ptr<gfx::GpuFence> TakeGpuFence(gfx::GpuFenceHandle fence)
{
    return fence.is_null() ? nullptr : std::make_unique<gfx::GpuFence>(std::move(fence));
}

} // namespace

NativeSkiaOutputDevice::NativeSkiaOutputDevice(
        scoped_refptr<gpu::SharedContextState> contextState, bool requiresAlpha,
        gpu::MemoryTracker *memoryTracker, viz::SkiaOutputSurfaceDependency *dependency,
        gpu::SharedImageFactory *shared_image_factory,
        gpu::SharedImageRepresentationFactory *shared_image_representation_factory,
        DidSwapBufferCompleteCallback didSwapBufferCompleteCallback)
    : SkiaOutputDevice(contextState->gr_context(), contextState->graphite_context(), memoryTracker,
                       didSwapBufferCompleteCallback)
    , Compositor(Type::Native)
    , m_grContextType(contextState->gr_context_type())
    , m_requiresAlpha(requiresAlpha)
    , m_factory(shared_image_factory)
    , m_representationFactory(shared_image_representation_factory)
    , m_deps(dependency)
{
    capabilities_.uses_default_gl_framebuffer = false;
    capabilities_.supports_surfaceless = true;
    capabilities_.output_surface_origin = gfx::SurfaceOrigin::kTopLeft;
    capabilities_.preserve_buffer_content = true;
    capabilities_.only_invalidates_damage_rect = false;
}

NativeSkiaOutputDevice::~NativeSkiaOutputDevice()
{
}

void NativeSkiaOutputDevice::SetFrameSinkId(const viz::FrameSinkId &id)
{
    bind(id);
}

bool NativeSkiaOutputDevice::Reshape(const SkImageInfo &image_info,
                                     const gfx::ColorSpace &colorSpace,
                                     int sample_count,
                                     float device_scale_factor,
                                     gfx::OverlayTransform transform)
{
    m_shape = Shape{image_info, device_scale_factor, colorSpace, sample_count};
    DCHECK_EQ(transform, gfx::OVERLAY_TRANSFORM_NONE);
    return true;
}

void NativeSkiaOutputDevice::Present(const absl::optional<gfx::Rect> &update_rect,
                                     BufferPresentedCallback feedback,
                                     viz::OutputSurfaceFrame frame)
{
    DCHECK(m_backBuffer);

    StartSwapBuffers(std::move(feedback));
    m_frame = std::move(frame);
    {
        QMutexLocker locker(&m_mutex);
        m_backBuffer->createFence();
        m_taskRunner = base::SingleThreadTaskRunner::GetCurrentDefault();
        std::swap(m_middleBuffer, m_backBuffer);
        m_readyToUpdate = true;
    }

    if (auto obs = observer())
        obs->readyToSwap();
}

void NativeSkiaOutputDevice::EnsureBackbuffer()
{
}

void NativeSkiaOutputDevice::DiscardBackbuffer()
{
}

SkSurface *NativeSkiaOutputDevice::BeginPaint(std::vector<GrBackendSemaphore> *end_semaphores)
{
    {
        QMutexLocker locker(&m_mutex);
        if (!m_backBuffer || m_backBuffer->shape() != m_shape) {
            m_backBuffer = std::make_unique<Buffer>(this);
            if (!m_backBuffer->initialize())
                return nullptr;
        }
    }
    auto surface = m_backBuffer->beginWriteSkia();
    *end_semaphores = m_backBuffer->takeEndWriteSkiaSemaphores();
    return surface;
}

void NativeSkiaOutputDevice::EndPaint()
{
    m_backBuffer->endWriteSkia(true);
}

void NativeSkiaOutputDevice::swapFrame()
{
    QMutexLocker locker(&m_mutex);
    if (m_readyToUpdate) {
        std::swap(m_frontBuffer, m_middleBuffer);
        m_taskRunner->PostTask(FROM_HERE,
                               base::BindOnce(&NativeSkiaOutputDevice::SwapBuffersFinished,
                                              base::Unretained(this)));
        m_taskRunner.reset();
        m_readyToUpdate = false;
        if (m_frontBuffer) {
            m_readyWithTexture = true;
            m_frontBuffer->beginPresent();
        }
        if (m_middleBuffer)
            m_middleBuffer->freeTexture();
    }
}

void NativeSkiaOutputDevice::waitForTexture()
{
    if (m_readyWithTexture)
        m_frontBuffer->consumeFence();
}

void NativeSkiaOutputDevice::releaseTexture()
{
    if (m_readyWithTexture) {
        m_frontBuffer->endPresent();
        m_readyWithTexture = false;
    }
}

void NativeSkiaOutputDevice::releaseResources()
{
    if (m_frontBuffer)
        m_frontBuffer->freeTexture();
}

bool NativeSkiaOutputDevice::textureIsFlipped()
{
    return false;
}

QSize NativeSkiaOutputDevice::size()
{
    return m_frontBuffer ? toQt(m_frontBuffer->shape().imageInfo.dimensions()) : QSize();
}

bool NativeSkiaOutputDevice::requiresAlphaChannel()
{
    return m_requiresAlpha;
}

float NativeSkiaOutputDevice::devicePixelRatio()
{
    return m_frontBuffer ? m_frontBuffer->shape().devicePixelRatio : 1;
}

void NativeSkiaOutputDevice::SwapBuffersFinished()
{
    {
        QMutexLocker locker(&m_mutex);
        std::swap(m_backBuffer, m_middleBuffer);
    }

    FinishSwapBuffers(gfx::SwapCompletionResult(gfx::SwapResult::SWAP_ACK),
                      gfx::Size(m_shape.imageInfo.width(), m_shape.imageInfo.height()),
                      std::move(m_frame));
}

NativeSkiaOutputDevice::Buffer::Buffer(NativeSkiaOutputDevice *parent)
    : m_parent(parent), m_shape(m_parent->m_shape)
{
}

NativeSkiaOutputDevice::Buffer::~Buffer()
{
    if (m_scopedSkiaWriteAccess)
        endWriteSkia(false);

    if (!m_mailbox.IsZero())
        m_parent->m_factory->DestroySharedImage(m_mailbox);
}

// The following Buffer methods are based on
// components/viz/service/display_embedder/output_presenter.cc: Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
bool NativeSkiaOutputDevice::Buffer::initialize()
{
    static const uint32_t kDefaultSharedImageUsage = gpu::SHARED_IMAGE_USAGE_SCANOUT
            | gpu::SHARED_IMAGE_USAGE_DISPLAY_READ | gpu::SHARED_IMAGE_USAGE_DISPLAY_WRITE
            | gpu::SHARED_IMAGE_USAGE_GLES2_FRAMEBUFFER_HINT;
    auto mailbox = gpu::Mailbox::GenerateForSharedImage();

    SkColorType skColorType = m_shape.imageInfo.colorType();
    if (!m_parent->m_factory->CreateSharedImage(
                mailbox, viz::SkColorTypeToSinglePlaneSharedImageFormat(skColorType),
                { m_shape.imageInfo.width(), m_shape.imageInfo.height() }, m_shape.colorSpace,
                kTopLeft_GrSurfaceOrigin, kPremul_SkAlphaType, m_parent->m_deps->GetSurfaceHandle(),
                kDefaultSharedImageUsage, "QWE_SharedImageBuffer")) {
        LOG(ERROR) << "CreateSharedImage failed.";
        return false;
    }
    m_mailbox = mailbox;

    m_skiaRepresentation = m_parent->m_representationFactory->ProduceSkia(
            m_mailbox, m_parent->m_deps->GetSharedContextState());
    if (!m_skiaRepresentation) {
        LOG(ERROR) << "ProduceSkia() failed.";
        return false;
    }

    m_overlayRepresentation = m_parent->m_representationFactory->ProduceOverlay(m_mailbox);
    if (!m_overlayRepresentation) {
        LOG(ERROR) << "ProduceOverlay() failed";
        return false;
    }

    return true;
}

SkSurface *NativeSkiaOutputDevice::Buffer::beginWriteSkia()
{
    DCHECK(!m_scopedSkiaWriteAccess);
    DCHECK(!m_presentCount);
    DCHECK(m_endSemaphores.empty());

    std::vector<GrBackendSemaphore> beginSemaphores;
    SkSurfaceProps surface_props{ 0, kUnknown_SkPixelGeometry };

    // Buffer queue is internal to GPU proc and handles texture initialization,
    // so allow uncleared access.
    m_scopedSkiaWriteAccess = m_skiaRepresentation->BeginScopedWriteAccess(
            m_shape.sampleCount, surface_props, &beginSemaphores, &m_endSemaphores,
            gpu::SharedImageRepresentation::AllowUnclearedAccess::kYes);
    DCHECK(m_scopedSkiaWriteAccess);
    if (!beginSemaphores.empty()) {
        m_scopedSkiaWriteAccess->surface()->wait(beginSemaphores.size(), beginSemaphores.data(),
                                                 /*deleteSemaphoresAfterWait=*/false);
    }
    return m_scopedSkiaWriteAccess->surface();
}

void NativeSkiaOutputDevice::Buffer::endWriteSkia(bool force_flush)
{
    // The Flush now takes place in finishPaintCurrentBuffer on the CPU side.
    // check if end_semaphores is not empty then flush here
    DCHECK(m_scopedSkiaWriteAccess);
    if (!m_endSemaphores.empty() || force_flush) {
        GrFlushInfo flush_info = {};
        flush_info.fNumSemaphores = m_endSemaphores.size();
        flush_info.fSignalSemaphores = m_endSemaphores.data();
        auto *direct_context =
                m_scopedSkiaWriteAccess->surface()->recordingContext()->asDirectContext();
        DCHECK(direct_context);
        direct_context->flush(m_scopedSkiaWriteAccess->surface(), {});
        m_scopedSkiaWriteAccess->ApplyBackendSurfaceEndState();
        direct_context->flush(m_scopedSkiaWriteAccess->surface(), flush_info, nullptr);
        direct_context->submit();
    }
    m_scopedSkiaWriteAccess.reset();
    m_endSemaphores.clear();

    // SkiaRenderer always draws the full frame.
    m_skiaRepresentation->SetCleared();
}

std::vector<GrBackendSemaphore> NativeSkiaOutputDevice::Buffer::takeEndWriteSkiaSemaphores()
{
    return std::exchange(m_endSemaphores, {});
}

void NativeSkiaOutputDevice::Buffer::beginPresent()
{
    if (++m_presentCount != 1) {
        DCHECK(m_scopedOverlayReadAccess);
        return;
    }

    DCHECK(!m_scopedSkiaWriteAccess);
    DCHECK(!m_scopedOverlayReadAccess);

    m_scopedOverlayReadAccess = m_overlayRepresentation->BeginScopedReadAccess();
    DCHECK(m_scopedOverlayReadAccess);
    m_acquireFence = TakeGpuFence(m_scopedOverlayReadAccess->TakeAcquireFence());
}

void NativeSkiaOutputDevice::Buffer::endPresent()
{
    if (!m_presentCount)
        return;
    DCHECK(m_scopedOverlayReadAccess);
    if (--m_presentCount)
        return;

    m_scopedOverlayReadAccess.reset();
}

void NativeSkiaOutputDevice::Buffer::freeTexture()
{
    if (textureCleanupCallback) {
        textureCleanupCallback();
        textureCleanupCallback = nullptr;
    }
}

void NativeSkiaOutputDevice::Buffer::createFence()
{
    // For some reason we still need to create this, but we do not need to wait on it.
    if (m_parent->m_grContextType == gpu::GrContextType::kGL)
        m_fence = gl::GLFence::Create();
}

void NativeSkiaOutputDevice::Buffer::consumeFence()
{
    if (m_acquireFence) {
        m_acquireFence->Wait();
        m_acquireFence.reset();
    }
}

#if defined(USE_OZONE)
scoped_refptr<gfx::NativePixmap> NativeSkiaOutputDevice::Buffer::nativePixmap()
{
    DCHECK(m_presentCount);
    return m_scopedOverlayReadAccess->GetNativePixmap();
}
#elif defined(Q_OS_WIN)
absl::optional<gl::DCLayerOverlayImage> NativeSkiaOutputDevice::Buffer::overlayImage() const
{
    DCHECK(m_presentCount);
    return m_scopedOverlayReadAccess->GetDCLayerOverlayImage();
}
#elif defined(Q_OS_MACOS)
gfx::ScopedIOSurface NativeSkiaOutputDevice::Buffer::ioSurface() const
{
    DCHECK(m_presentCount);
    return m_scopedOverlayReadAccess->GetIOSurface();
}
#endif

} // namespace QtWebEngineCore
