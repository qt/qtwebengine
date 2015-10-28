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
#ifndef RENDER_VIEW_OBSERVER_QT_H
#define RENDER_VIEW_OBSERVER_QT_H

#include "content/public/renderer/render_view_observer.h"

#include <QtGlobal>

namespace web_cache {
class WebCacheRenderProcessObserver;
}

class RenderViewObserverQt : public content::RenderViewObserver {
public:
    RenderViewObserverQt(content::RenderView* render_view,
                         web_cache::WebCacheRenderProcessObserver* web_cache_render_process_observer);

private:
    void onFetchDocumentMarkup(quint64 requestId);
    void onFetchDocumentInnerText(quint64 requestId);
    void onSetBackgroundColor(quint32 color);

    void OnFirstVisuallyNonEmptyLayout() Q_DECL_OVERRIDE;

    virtual bool OnMessageReceived(const IPC::Message& message) Q_DECL_OVERRIDE;
    virtual void Navigate(const GURL& url) Q_DECL_OVERRIDE;

    web_cache::WebCacheRenderProcessObserver* m_web_cache_render_process_observer;

    DISALLOW_COPY_AND_ASSIGN(RenderViewObserverQt);
};

#endif // RENDER_VIEW_OBSERVER_QT_H
