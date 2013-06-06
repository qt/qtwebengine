#ifndef WEB_CONTENTS_DELEGATE_QT
#define WEB_CONTENTS_DELEGATE_QT

#include "content/public/browser/notification_observer.h"
#include "content/public/browser/web_contents_delegate.h"

namespace content {
    class BrowserContext;
    class SiteInstance;
}

class QWebContentsView;
class QQuickWebContentsView;

class WebContentsDelegateQt : public content::WebContentsDelegate
                            , public content::NotificationObserver
{
public:
    static WebContentsDelegateQt* Create(content::WebContents*, const gfx::Size& initial_size, QWebContentsView*);
    static WebContentsDelegateQt* Create(content::WebContents*, const gfx::Size& initial_size, QQuickWebContentsView*);
    static WebContentsDelegateQt* CreateNewWindow(content::BrowserContext*, const GURL&, content::SiteInstance*, int routing_id, const gfx::Size& initial_size, QWebContentsView*);
    static WebContentsDelegateQt* CreateNewWindow(content::BrowserContext*, const GURL&, content::SiteInstance*, int routing_id, const gfx::Size& initial_size, QQuickWebContentsView*);
    content::WebContents* web_contents();
    void LoadURL(const GURL&);
    void LoadURLForFrame(const GURL&, const std::string& frame_name);
    void GoBackOrForward(int offset);
    void Reload();
    void Stop();

    virtual void Observe(int, const content::NotificationSource&, const content::NotificationDetails&);

private:
    static WebContentsDelegateQt* commonCreate(content::WebContents* web_contents, const gfx::Size& initial_size, WebContentsDelegateQt* delegate);
    WebContentsDelegateQt(content::WebContents*, QWebContentsView*);
    WebContentsDelegateQt(content::WebContents*, QQuickWebContentsView*);
    void PlatformCreateWindow(int width, int height);
    void PlatformSetContents();
    void PlatformResizeSubViews();

    QWebContentsView* m_contentsView;
    QQuickWebContentsView* m_quickContentsView;
    scoped_ptr<content::WebContents> m_webContents;

    static std::vector<WebContentsDelegateQt*> m_windows;
};

#endif
