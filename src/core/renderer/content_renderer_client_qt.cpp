/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "renderer/content_renderer_client_qt.h"

#include "base/strings/utf_string_conversions.h"
#include "content/public/renderer/render_thread.h"
#include "net/base/net_errors.h"
#include "third_party/WebKit/public/platform/WebURLError.h"
#include "third_party/WebKit/public/platform/WebURLRequest.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/webui/jstemplate_builder.h"

#include "common/localized_error.h"
#include "renderer/qt_render_view_observer.h"

#include "grit/renderer_resources.h"

static const char kHttpErrorDomain[] = "http";

void ContentRendererClientQt::RenderViewCreated(content::RenderView* render_view)
{
    // RenderViewObserver destroys itself with its RenderView.
    new QtRenderViewObserver(render_view);
}

// To tap into the chromium localized strings. Ripped from the chrome layer (highly simplified).
void ContentRendererClientQt::GetNavigationErrorStrings(blink::WebFrame *frame, const blink::WebURLRequest &failed_request, const blink::WebURLError &error, const std::string &accept_languages, std::string *error_html, base::string16 *error_description)
{
    Q_UNUSED(frame)

    const bool isPost = EqualsASCII(failed_request.httpMethod(), "POST");

    if (error_html) {
      // Use a local error page.
      int resource_id;
      base::DictionaryValue error_strings;

      const std::string locale = content::RenderThread::Get()->GetLocale();
      /* FIXME: rip that as well ?
        if (!NetErrorHelper::GetErrorStringsForDnsProbe(
                frame, error, is_post, locale, accept_languages,
                &error_strings)) {
          // In most cases, the NetErrorHelper won't provide DNS-probe-specific
          // error pages, so fall back to LocalizedError.
*/
      LocalizedError::GetStrings(error.reason, error.domain.utf8(),
                                 error.unreachableURL, isPost, locale,
                                 accept_languages, &error_strings);
      resource_id = IDR_NET_ERROR_HTML;


      const base::StringPiece template_html(ui::ResourceBundle::GetSharedInstance().GetRawDataResource(resource_id));
      if (template_html.empty())
        NOTREACHED() << "unable to load template. ID: " << resource_id;
      else // "t" is the id of the templates root node.
        *error_html = webui::GetTemplatesHtml(template_html, &error_strings, "t");
    }

    if (error_description) {
        *error_description = LocalizedError::GetErrorDetails(error, isPost);
    }
}
