#ifndef WEB_CONTENTS_VIEW_QT_
#define WEB_CONTENTS_VIEW_QT_

#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/web_contents/web_contents_view_gtk.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "render_widget_host_view_qt.h"

class WebContentsViewQt : public content::WebContentsViewGtk
{
public:
    WebContentsViewQt(content::WebContents* web_contents)
        : content::WebContentsViewGtk(static_cast<content::WebContentsImpl*>(web_contents), 0)
    { }

    content::RenderWidgetHostView* CreateViewForWidget(content::RenderWidgetHost* render_widget_host)
    {
        content::RenderWidgetHostView* view = content::RenderWidgetHostView::CreateViewForWidget(render_widget_host);
        view->InitAsChild(reinterpret_cast<gfx::NativeView>(m_windowContainer));

        return view;
    }

    void setWindowContainer(void* c) { m_windowContainer = c; }
    void* windowContainer() { return m_windowContainer; }

private:
    void* m_windowContainer;
};

#endif