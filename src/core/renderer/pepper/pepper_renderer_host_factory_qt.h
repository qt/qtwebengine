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

#ifndef PEPPER_RENDERER_HOST_FACTORY_QT_H
#define PEPPER_RENDERER_HOST_FACTORY_QT_H

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ppapi/host/host_factory.h"
#include "content/public/renderer/render_frame_observer.h"


namespace content {
class RenderFrame;
}

namespace QtWebEngineCore {

class PepperRendererHostFactoryQt : public ppapi::host::HostFactory {
public:
    explicit PepperRendererHostFactoryQt(content::RendererPpapiHost* host);
    ~PepperRendererHostFactoryQt();

    // HostFactory.
    scoped_ptr<ppapi::host::ResourceHost> CreateResourceHost(
            ppapi::host::PpapiHost* host,
            PP_Resource resource,
            PP_Instance instance,
            const IPC::Message& message) override;

private:
    // Not owned by this object.
    content::RendererPpapiHost* host_;

    DISALLOW_COPY_AND_ASSIGN(PepperRendererHostFactoryQt);
};

} // namespace QtWebEngineCore

#endif // PEPPER_RENDERER_HOST_FACTORY_QT_H
