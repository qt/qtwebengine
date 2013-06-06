#include "web_contents_delegate_qt.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/navigation_controller.h"
#include "web_contents_view_qt.h"
#include "qwebcontentsview.h"
#include "qquickwebcontentsview.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLineEdit>
#include <QQuickView>
#include <QApplication>

static const int kTestWindowWidth = 800;
static const int kTestWindowHeight = 600;

std::vector<WebContentsDelegateQt*> WebContentsDelegateQt::m_windows;

WebContentsDelegateQt::WebContentsDelegateQt(content::WebContents* web_contents, QWebContentsView* contentsView)
    : m_contentsView(contentsView)
    , m_quickContentsView(NULL)
{
	// const CommandLine& command_line = *CommandLine::ForCurrentProcess();
	// registrar_.Add(this, NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
	// Source<WebContents>(web_contents));
	m_windows.push_back(this);

	// if (!shell_created_callback_.is_null()) {
	// 	shell_created_callback_.Run(this);
	// 	shell_created_callback_.Reset();
	// }
}

WebContentsDelegateQt::WebContentsDelegateQt(content::WebContents* web_contents, QQuickWebContentsView* contentsView)
    : m_contentsView(NULL)
    , m_quickContentsView(contentsView)
{
    // const CommandLine& command_line = *CommandLine::ForCurrentProcess();
    // registrar_.Add(this, NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
    // Source<WebContents>(web_contents));
    m_windows.push_back(this);

    // if (!shell_created_callback_.is_null()) {
    //  shell_created_callback_.Run(this);
    //  shell_created_callback_.Reset();
    // }
}

WebContentsDelegateQt* WebContentsDelegateQt::commonCreate(content::WebContents* web_contents, const gfx::Size& initial_size, WebContentsDelegateQt* delegate)
{
    delegate->PlatformCreateWindow(initial_size.width(), initial_size.height());

    delegate->m_webContents.reset(web_contents);
    web_contents->SetDelegate(delegate);

    delegate->PlatformSetContents();
    delegate->PlatformResizeSubViews();

    return delegate; 
}

content::WebContents* commonCreateWebContents(content::WebContents::CreateParams& create_params, int routing_id, const gfx::Size& initial_size)
{
    create_params.routing_id = routing_id;
    if (!initial_size.IsEmpty())
        create_params.initial_size = initial_size;
    else
        create_params.initial_size = gfx::Size(kTestWindowWidth, kTestWindowHeight);
    return content::WebContents::Create(create_params);
}

WebContentsDelegateQt* WebContentsDelegateQt::Create(content::WebContents* web_contents, const gfx::Size& initial_size, QWebContentsView* contentsView)
{
    WebContentsDelegateQt* delegate = new WebContentsDelegateQt(web_contents, contentsView);
    return commonCreate(web_contents, initial_size, delegate);
}

WebContentsDelegateQt* WebContentsDelegateQt::Create(content::WebContents* web_contents, const gfx::Size& initial_size, QQuickWebContentsView* contentsView)
{
	WebContentsDelegateQt* delegate = new WebContentsDelegateQt(web_contents, contentsView);
    return commonCreate(web_contents, initial_size, delegate);
}

WebContentsDelegateQt* WebContentsDelegateQt::CreateNewWindow(content::BrowserContext* browser_context, const GURL& url, content::SiteInstance* site_instance, int routing_id, const gfx::Size& initial_size, QWebContentsView* contentsView)
{
    content::WebContents::CreateParams create_params(browser_context, site_instance);
    content::WebContents* web_contents = commonCreateWebContents(create_params, routing_id, initial_size);
    WebContentsDelegateQt* contentsDelegate = Create(web_contents, create_params.initial_size, contentsView);
    if (!url.is_empty())
        contentsDelegate->LoadURL(url);
    return contentsDelegate;
}

WebContentsDelegateQt* WebContentsDelegateQt::CreateNewWindow(content::BrowserContext* browser_context, const GURL& url, content::SiteInstance* site_instance, int routing_id, const gfx::Size& initial_size, QQuickWebContentsView* contentsView)
{
    content::WebContents::CreateParams create_params(browser_context, site_instance);
    content::WebContents* web_contents = commonCreateWebContents(create_params, routing_id, initial_size);
    WebContentsDelegateQt* contentsDelegate = Create(web_contents, create_params.initial_size, contentsView);
    if (!url.is_empty())
        contentsDelegate->LoadURL(url);
    return contentsDelegate;
}

content::WebContents* WebContentsDelegateQt::web_contents()
{
    return m_webContents.get();
}

void WebContentsDelegateQt::PlatformCreateWindow(int width, int height)
{
    if (m_contentsView) {
        // The layout is used in PlatformSetContents.
        QVBoxLayout* layout = new QVBoxLayout;
        m_contentsView->setLayout(layout);
    }
}

void WebContentsDelegateQt::PlatformSetContents()
{
    content::RendererPreferences* rendererPrefs = m_webContents->GetMutableRendererPrefs();
    rendererPrefs->use_custom_colors = true;
    // Qt returns a flash time (the whole cycle) in ms, chromium expects just the interval in seconds
    rendererPrefs->caret_blink_interval = static_cast<double>(qApp->cursorFlashTime())/2000;
    m_webContents->GetRenderViewHost()->SyncRendererPrefs();

    if (m_contentsView) {
        WebContentsViewQt* content_view = static_cast<WebContentsViewQt*>(m_webContents->GetView());
        QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(m_contentsView->layout());
        if (layout)
            layout->addLayout(content_view->windowContainer()->widget());
    } else if (m_quickContentsView) {
        WebContentsViewQt* content_view = static_cast<WebContentsViewQt*>(m_webContents->GetView());
        QQuickItem* windowContainer = content_view->windowContainer()->qQuickItem();
        windowContainer->setParentItem(m_quickContentsView);
    }
}

void WebContentsDelegateQt::PlatformResizeSubViews()
{

}

void WebContentsDelegateQt::GoBackOrForward(int offset) {
  m_webContents->GetController().GoToOffset(offset);
  m_webContents->GetView()->Focus();
}

void WebContentsDelegateQt::Reload() {
  m_webContents->GetController().Reload(false);
  m_webContents->GetView()->Focus();
}

void WebContentsDelegateQt::Stop() {
  m_webContents->Stop();
  m_webContents->GetView()->Focus();
}

void WebContentsDelegateQt::LoadURL(const GURL& url)
{
  	LoadURLForFrame(url, std::string());
}

void WebContentsDelegateQt::LoadURLForFrame(const GURL& url, const std::string& frame_name) {
  content::NavigationController::LoadURLParams params(url);
  params.transition_type = content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
  params.frame_name = frame_name;
  m_webContents->GetController().LoadURLWithParams(params);
  m_webContents->GetView()->Focus();
}

void WebContentsDelegateQt::Observe(int, const content::NotificationSource&, const content::NotificationDetails&)
{
	// IMPLEMENT THIS!!!!
}
