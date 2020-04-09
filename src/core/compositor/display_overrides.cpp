/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "display_gl_output_surface.h"
#include "display_software_output_surface.h"

#include "components/viz/service/display_embedder/output_surface_provider_impl.h"
#include "gpu/ipc/in_process_command_buffer.h"
#include <qtgui-config.h>

std::unique_ptr<viz::OutputSurface>
viz::OutputSurfaceProviderImpl::CreateGLOutputSurface(
        scoped_refptr<VizProcessContextProvider> context_provider)
{
#if QT_CONFIG(opengl)
    return std::make_unique<QtWebEngineCore::DisplayGLOutputSurface>(std::move(context_provider));
#else
    return nullptr;
#endif // QT_CONFIG(opengl)
}

std::unique_ptr<viz::OutputSurface>
viz::OutputSurfaceProviderImpl::CreateSoftwareOutputSurface()
{
    return std::make_unique<QtWebEngineCore::DisplaySoftwareOutputSurface>();
}

void gpu::InProcessCommandBuffer::GetTextureQt(
        unsigned int client_id,
        GetTextureCallback callback,
        const std::vector<SyncToken>& sync_token_fences)
{
    ScheduleGpuTask(base::BindOnce(&InProcessCommandBuffer::GetTextureQtOnGpuThread,
                                   gpu_thread_weak_ptr_factory_.GetWeakPtr(),
                                   client_id,
                                   std::move(callback)),
                    sync_token_fences);
}

void gpu::InProcessCommandBuffer::GetTextureQtOnGpuThread(
        unsigned int client_id, GetTextureCallback callback)
{
    if (!MakeCurrent()) {
        LOG(ERROR) << "MakeCurrent failed for GetTextureQt";
        return;
    }
    gpu::TextureBase *texture = decoder_->GetTextureBase(client_id);
    std::move(callback).Run(texture ? texture->service_id() : 0, gl::GLFence::Create());
}
