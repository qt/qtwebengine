// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "media_capture_devices_dispatcher.h"

#include "type_conversion.h"
#include "web_contents_delegate_qt.h"
#include "web_contents_view_qt.h"
#include "web_engine_settings.h"

#include "base/strings/strcat.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/desktop_media_id.h"
#include "content/public/browser/desktop_streams_registry.h"
#include "content/public/browser/media_capture_devices.h"
#include "content/public/browser/render_process_host.h"
#include "media/audio/audio_device_description.h"
#include "media/audio/audio_manager_base.h"
#include "services/network/public/cpp/is_potentially_trustworthy.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom-shared.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom.h"

#if QT_CONFIG(webengine_webrtc)
#include "third_party/webrtc/modules/desktop_capture/desktop_capture_options.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_capturer.h"
#endif

#if defined(WEBRTC_USE_X11)
#include <dlfcn.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xlib.h>
#endif

namespace QtWebEngineCore {

using content::BrowserThread;
using blink::mojom::MediaStreamRequestResult;
using blink::mojom::MediaStreamType;

namespace {

const blink::MediaStreamDevice *findDeviceWithId(const blink::MediaStreamDevices &devices, const std::string &deviceId)
{
    blink::MediaStreamDevices::const_iterator iter = devices.begin();
    for (; iter != devices.end(); ++iter) {
        if (iter->id == deviceId) {
            return &(*iter);
        }
    }
    return nullptr;
}

// Based on chrome/browser/media/webrtc/desktop_capture_devices_util.cc:
media::mojom::CaptureHandlePtr CreateCaptureHandle(content::WebContents *capturer,
                                                   const url::Origin &capturer_origin,
                                                   const content::DesktopMediaID &captured_id)
{
    if (capturer_origin.opaque())
        return nullptr;

    content::RenderFrameHost *const captured_rfh =
            content::RenderFrameHost::FromID(
                captured_id.web_contents_id.render_process_id,
                captured_id.web_contents_id.main_render_frame_id);
    if (!captured_rfh || !captured_rfh->IsActive())
        return nullptr;

    content::WebContents *const captured =  content::WebContents::FromRenderFrameHost(captured_rfh);
    if (!captured)
        return nullptr;

    const auto &captured_config = captured->GetCaptureHandleConfig();
    if (!captured_config.all_origins_permitted &&
            std::none_of(captured_config.permitted_origins.begin(),
                         captured_config.permitted_origins.end(),
                         [capturer_origin](const url::Origin& permitted_origin) {
                            return capturer_origin.IsSameOriginWith(permitted_origin);
                         }))
    {
        return nullptr;
    }

    // Observing CaptureHandle when either the capturing or the captured party
    // is incognito is disallowed, except for self-capture.
    if (capturer->GetPrimaryMainFrame() != captured->GetPrimaryMainFrame()) {
        if (capturer->GetBrowserContext()->IsOffTheRecord() ||
            captured->GetBrowserContext()->IsOffTheRecord()) {
            return nullptr;
        }
    }

    if (!captured_config.expose_origin && captured_config.capture_handle.empty())
        return nullptr;

    auto result = media::mojom::CaptureHandle::New();
    if (captured_config.expose_origin)
        result->origin = captured->GetPrimaryMainFrame()->GetLastCommittedOrigin();

    result->capture_handle = captured_config.capture_handle;

    return result;
}

// Based on chrome/browser/media/webrtc/desktop_capture_devices_util.cc:
media::mojom::DisplayMediaInformationPtr DesktopMediaIDToDisplayMediaInformation(content::WebContents *capturer,
                                                                                 const url::Origin &capturer_origin,
                                                                                 const content::DesktopMediaID &media_id)
{
    media::mojom::DisplayCaptureSurfaceType display_surface = media::mojom::DisplayCaptureSurfaceType::MONITOR;
    bool logical_surface = true;
    media::mojom::CursorCaptureType cursor = media::mojom::CursorCaptureType::NEVER;
#if defined(USE_AURA)
    const bool uses_aura = (media_id.window_id != content::DesktopMediaID::kNullId ? true : false);
#else
    const bool uses_aura = false;
#endif  // defined(USE_AURA)

    media::mojom::CaptureHandlePtr capture_handle;
    switch (media_id.type) {
    case content::DesktopMediaID::TYPE_SCREEN:
        display_surface = media::mojom::DisplayCaptureSurfaceType::MONITOR;
        cursor = uses_aura ? media::mojom::CursorCaptureType::MOTION
                           : media::mojom::CursorCaptureType::ALWAYS;
        break;
    case content::DesktopMediaID::TYPE_WINDOW:
        display_surface = media::mojom::DisplayCaptureSurfaceType::WINDOW;
        cursor = uses_aura ? media::mojom::CursorCaptureType::MOTION
                           : media::mojom::CursorCaptureType::ALWAYS;
        break;
    case content::DesktopMediaID::TYPE_WEB_CONTENTS:
        display_surface = media::mojom::DisplayCaptureSurfaceType::BROWSER;
        cursor = media::mojom::CursorCaptureType::MOTION;
        capture_handle = CreateCaptureHandle(capturer, capturer_origin, media_id);
        break;
    case content::DesktopMediaID::TYPE_NONE:
        break;
    }

    return media::mojom::DisplayMediaInformation::New(display_surface, logical_surface, cursor, std::move(capture_handle));
}


// Based on chrome/browser/media/webrtc/desktop_capture_devices_util.cc:
std::string DeviceNamePrefix(content::WebContents *web_contents,
                             blink::mojom::MediaStreamType requested_stream_type,
                             const content::DesktopMediaID &media_id)
{
    if (!web_contents || requested_stream_type != blink::mojom::MediaStreamType::DISPLAY_VIDEO_CAPTURE_THIS_TAB) {
        return std::string();
    }

    // Note that all of these must still be checked, as the explicit-selection
    // dialog for DISPLAY_VIDEO_CAPTURE_THIS_TAB could still return something
    // other than the current tab - be it a screen, window, or another tab.
    if (media_id.type == content::DesktopMediaID::TYPE_WEB_CONTENTS &&
            web_contents->GetPrimaryMainFrame()->GetProcess()->GetID() ==
                media_id.web_contents_id.render_process_id &&
            web_contents->GetPrimaryMainFrame()->GetRoutingID() ==
                media_id.web_contents_id.main_render_frame_id) {
        return "current-";
    }

    return std::string();
}

// Based on chrome/browser/media/webrtc/desktop_capture_devices_util.cc:
std::string DeviceName(content::WebContents *web_contents,
                       blink::mojom::MediaStreamType requested_stream_type,
                       const content::DesktopMediaID &media_id)
{
    const std::string prefix =
        DeviceNamePrefix(web_contents, requested_stream_type, media_id);
    if (media_id.type == content::DesktopMediaID::TYPE_WEB_CONTENTS) {
        return base::StrCat({prefix, content::kWebContentsCaptureScheme,
                             base::UnguessableToken::Create().ToString()});
    } else {
        // TODO(crbug.com/1252682): MediaStreamTrack.label leaks internal state for
        // screen/window
        return base::StrCat({prefix, media_id.ToString()});
    }
}

// Based on chrome/browser/media/webrtc/desktop_capture_devices_util.cc:
void getDevicesForDesktopCapture(const content::MediaStreamRequest &request,
                                 content::WebContents *web_contents,
                                 content::DesktopMediaID mediaId,
                                 bool captureAudio,
                                 bool disableLocalEcho,
                                 blink::mojom::StreamDevices &out_devices)
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);

    // Add selected desktop source to the list.
    blink::MediaStreamDevice device(request.video_type, mediaId.ToString(),
                                    DeviceName(web_contents, request.video_type, mediaId));
    device.display_media_info = DesktopMediaIDToDisplayMediaInformation(
                web_contents, url::Origin::Create(request.security_origin), mediaId);
    out_devices.video_device = device;

    if (captureAudio) {
        DCHECK_NE(request.audio_type, blink::mojom::MediaStreamType::NO_SERVICE);

        if (mediaId.type == content::DesktopMediaID::TYPE_WEB_CONTENTS) {
            content::WebContentsMediaCaptureId web_id = mediaId.web_contents_id;
            web_id.disable_local_echo = disableLocalEcho;
            out_devices.audio_device = blink::MediaStreamDevice(request.audio_type, web_id.ToString(), "Tab audio");
        } else {
            // Use the special loopback device ID for system audio capture.
            out_devices.audio_device = blink::MediaStreamDevice(
                        request.audio_type, (disableLocalEcho
                                 ? media::AudioDeviceDescription::kLoopbackWithMuteDeviceId
                                 : media::AudioDeviceDescription::kLoopbackInputDeviceId),
                        "System Audio");
        }
    }
}

content::DesktopMediaID getDefaultScreenId()
{
#if QT_CONFIG(webengine_webrtc)
    // Source id patterns are different across platforms.
    // On Linux and macOS, the source ids are randomish numbers assigned by the OS.
    // On Windows, the screens are enumerated consecutively in increasing order from 0.

    // In order to provide a correct screen id, we query for the available screen ids, and
    // select the first one as the main display id.
#if !defined(WEBRTC_USE_X11)
    // The code is based on the file
    // chrome/browser/media/webrtc/native_desktop_media_list.cc.
    webrtc::DesktopCaptureOptions options =
            webrtc::DesktopCaptureOptions::CreateDefault();
    options.set_disable_effects(false);
    std::unique_ptr<webrtc::DesktopCapturer> screen_capturer(
            webrtc::DesktopCapturer::CreateScreenCapturer(options));

    if (screen_capturer) {
        webrtc::DesktopCapturer::SourceList screens;
        if (screen_capturer->GetSourceList(&screens)) {
            if (screens.size() > 0) {
                return content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, screens[0].id);
            }
        }
    }
#else
    // This is a workaround to avoid thread issues with DesktopCapturer [1]. Unfortunately,
    // creating a DesktopCapturer is not thread safe on X11 due to the use of webrtc::XErrorTrap.
    // Can be removed if https://crbug.com/2022 and/or https://crbug.com/570852 are fixed.
    // The code is based on the file
    // third_party/webrtc/modules/desktop_capture/linux/screen_capturer_x11.cc.
    //
    // [1]: webrtc::InProcessVideoCaptureDeviceLauncher::DoStartDesktopCaptureOnDeviceThread
    Display *display = XOpenDisplay(nullptr);
    if (!display) {
        qWarning("Unable to open display.");
        return content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, 0);
    }

    int randrEventBase = 0;
    int errorBaseIgnored = 0;
    if (!XRRQueryExtension(display, &randrEventBase, &errorBaseIgnored)) {
        qWarning("X server does not support XRandR.");
        return content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, 0);
    }

    int majorVersion = 0;
    int minorVersion = 0;
    if (!XRRQueryVersion(display, &majorVersion, &minorVersion)) {
        qWarning("X server does not support XRandR.");
        return content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, 0);
    }

    if (majorVersion < 1 || (majorVersion == 1 && minorVersion < 5)) {
        qWarning("XRandR entension is older than v1.5.");
        return content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, 0);
    }

    typedef XRRMonitorInfo *(*GetMonitorsFunc)(Display *, Window, Bool, int *);
    GetMonitorsFunc getMonitors = reinterpret_cast<GetMonitorsFunc>(dlsym(RTLD_DEFAULT, "XRRGetMonitors"));
    typedef void (*FreeMonitorsFunc)(XRRMonitorInfo*);
    FreeMonitorsFunc freeMonitors = reinterpret_cast<FreeMonitorsFunc>(dlsym(RTLD_DEFAULT, "XRRFreeMonitors"));
    if (!getMonitors || !freeMonitors) {
        qWarning("Unable to link XRandR monitor functions.");
        return content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, 0);
    }

    Window rootWindow = RootWindow(display, DefaultScreen(display));
    if (rootWindow == BadValue) {
        qWarning("Unable to get the root window.");
        return content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, 0);
    }

    int numMonitors = 0;
    XRRMonitorInfo *monitors = getMonitors(display, rootWindow, true, &numMonitors);
    auto cleanup = qScopeGuard([&] () { freeMonitors(monitors); });
    if (numMonitors > 0)
        return content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, monitors[0].name);
#endif // !defined(WEBRTC_USE_X11)
#endif // QT_CONFIG(webengine_webrtc)

    return content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, 0);
}

WebContentsAdapterClient::MediaRequestFlags mediaRequestFlagsForRequest(const content::MediaStreamRequest &request)
{
    if (request.audio_type == MediaStreamType::DEVICE_AUDIO_CAPTURE &&
        request.video_type == MediaStreamType::DEVICE_VIDEO_CAPTURE)
        return {WebContentsAdapterClient::MediaAudioCapture, WebContentsAdapterClient::MediaVideoCapture};

    if (request.audio_type == MediaStreamType::DEVICE_AUDIO_CAPTURE &&
        request.video_type == MediaStreamType::NO_SERVICE)
        return {WebContentsAdapterClient::MediaAudioCapture};

    if (request.audio_type == MediaStreamType::NO_SERVICE &&
        request.video_type == MediaStreamType::DEVICE_VIDEO_CAPTURE)
        return {WebContentsAdapterClient::MediaVideoCapture};

    if (request.audio_type == MediaStreamType::GUM_DESKTOP_AUDIO_CAPTURE &&
        request.video_type == MediaStreamType::GUM_DESKTOP_VIDEO_CAPTURE)
        return {WebContentsAdapterClient::MediaDesktopAudioCapture, WebContentsAdapterClient::MediaDesktopVideoCapture};

    if (request.audio_type == MediaStreamType::DISPLAY_AUDIO_CAPTURE &&
        request.video_type == MediaStreamType::DISPLAY_VIDEO_CAPTURE)
        return {WebContentsAdapterClient::MediaDesktopAudioCapture, WebContentsAdapterClient::MediaDesktopVideoCapture};

    if (request.audio_type == MediaStreamType::DISPLAY_AUDIO_CAPTURE &&
        request.video_type == MediaStreamType::DISPLAY_VIDEO_CAPTURE_THIS_TAB)
        return {WebContentsAdapterClient::MediaDesktopAudioCapture, WebContentsAdapterClient::MediaDesktopVideoCapture};

    if (request.video_type == MediaStreamType::GUM_DESKTOP_VIDEO_CAPTURE ||
        request.video_type == MediaStreamType::DISPLAY_VIDEO_CAPTURE ||
        request.video_type == MediaStreamType::DISPLAY_VIDEO_CAPTURE_THIS_TAB)
        return {WebContentsAdapterClient::MediaDesktopVideoCapture};

    return {};
}

// Based on MediaStreamCaptureIndicator::UIDelegate
class MediaStreamUIQt : public content::MediaStreamUI
{
public:
    MediaStreamUIQt(content::WebContents *webContents, const blink::mojom::StreamDevices &devices)
        : m_delegate(static_cast<WebContentsDelegateQt *>(webContents->GetDelegate())->AsWeakPtr())
        , m_devices(devices)
    {
        DCHECK(m_devices.audio_device.has_value() ||
               m_devices.video_device.has_value());
    }

    ~MediaStreamUIQt() override
    {
        if (m_started && m_delegate)
            m_delegate->removeDevices(m_devices);
        m_onStop.Reset();
    }

private:
    gfx::NativeViewId OnStarted(base::RepeatingClosure stop, SourceCallback source,
                                const std::string& label,
                                std::vector<content::DesktopMediaID> screen_capture_ids,
                                StateChangeCallback state_change) override
    {
        if (m_started) {
            // Ignore possibly-compromised renderers that might call
            // MediaStreamDispatcherHost::OnStreamStarted() more than once.
            // See: https://crbug.com/1155426
            return 0;
        }
        m_started = true;
        m_onStop = std::move(stop);
        if (m_delegate)
            m_delegate->addDevices(m_devices);
        return 0;
    }
    void OnDeviceStopped(const std::string &label, const content::DesktopMediaID &media_id) override
    {
        Q_UNUSED(label);
        Q_UNUSED(media_id);
    }
    void OnDeviceStoppedForSourceChange(const std::string&, const content::DesktopMediaID&, const content::DesktopMediaID&) override
    {}
    base::WeakPtr<WebContentsDelegateQt> m_delegate;
    const blink::mojom::StreamDevices m_devices;
    bool m_started = false;
    base::RepeatingClosure m_onStop; // currently unused
};

} // namespace

MediaCaptureDevicesDispatcher::PendingAccessRequest::PendingAccessRequest(const content::MediaStreamRequest &request,
                                                                          content::MediaResponseCallback callback)
        : request(request)
        , callback(std::move(callback))
{
}

MediaCaptureDevicesDispatcher::PendingAccessRequest::~PendingAccessRequest()
{
}

void MediaCaptureDevicesDispatcher::handleMediaAccessPermissionResponse(content::WebContents *webContents, const QUrl &securityOrigin, WebContentsAdapterClient::MediaRequestFlags authorizationFlags)
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);

    blink::mojom::StreamDevicesSet deviceSet;
    auto it = m_pendingRequests.find(webContents);
    if (it == m_pendingRequests.end() || it->second.empty())
        return;

    RequestsQueue &queue(it->second);
    content::MediaStreamRequest &request = queue.front()->request;

    const QUrl requestSecurityOrigin(toQt(request.security_origin));
    bool securityOriginsMatch = (requestSecurityOrigin.host() == securityOrigin.host()
                                 && requestSecurityOrigin.scheme() == securityOrigin.scheme()
                                 && requestSecurityOrigin.port() == securityOrigin.port());
    if (!securityOriginsMatch)
        qWarning("Security origin mismatch for media access permission: %s requested and %s provided\n", qPrintable(requestSecurityOrigin.toString()), qPrintable(securityOrigin.toString()));

    WebContentsAdapterClient::MediaRequestFlags requestFlags = mediaRequestFlagsForRequest(request);
    WebContentsAdapterClient::MediaRequestFlags finalFlags = requestFlags & authorizationFlags;

    bool microphoneRequested = finalFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture);
    bool webcamRequested = finalFlags.testFlag(WebContentsAdapterClient::MediaVideoCapture);
    bool desktopAudioRequested = finalFlags.testFlag(WebContentsAdapterClient::MediaDesktopAudioCapture);
    bool desktopVideoRequested = finalFlags.testFlag(WebContentsAdapterClient::MediaDesktopVideoCapture);

    if (securityOriginsMatch) {
        if (microphoneRequested || webcamRequested) {
            switch (request.request_type) {
            case blink::MEDIA_OPEN_DEVICE_PEPPER_ONLY:
                getDefaultDevices("", "", microphoneRequested, webcamRequested, deviceSet);
                break;
            case blink::MEDIA_DEVICE_ACCESS:
            case blink::MEDIA_DEVICE_UPDATE:
            case blink::MEDIA_GENERATE_STREAM:
            case blink::MEDIA_GET_OPEN_DEVICE:
                getDefaultDevices(request.requested_audio_device_id, request.requested_video_device_id,
                                  microphoneRequested, webcamRequested, deviceSet);
                break;
            }
        } else if (desktopVideoRequested) {
            deviceSet.stream_devices.emplace_back(blink::mojom::StreamDevices::New());
            bool captureAudio = desktopAudioRequested && m_loopbackAudioSupported;
            blink::mojom::StreamDevices &stream_devices = *deviceSet.stream_devices[0];
            getDevicesForDesktopCapture(request, webContents, getDefaultScreenId(), captureAudio,
                                        request.disable_local_echo, stream_devices);
        }
    }

    content::MediaResponseCallback callback = std::move(queue.front()->callback);
    queue.pop_front();

    if (!queue.empty()) {
        // Post a task to process next queued request. It has to be done
        // asynchronously to make sure that calling infobar is not destroyed until
        // after this function returns.
        content::GetUIThreadTaskRunner({})->PostTask(FROM_HERE,
                       base::BindOnce(&MediaCaptureDevicesDispatcher::ProcessQueuedAccessRequest,
                                      base::Unretained(this), webContents));
    }

    if (deviceSet.stream_devices.empty())
        std::move(callback).Run(deviceSet, MediaStreamRequestResult::INVALID_STATE,
                                std::unique_ptr<content::MediaStreamUI>());
    else
        std::move(callback).Run(deviceSet, MediaStreamRequestResult::OK,
                                std::make_unique<MediaStreamUIQt>(webContents, *deviceSet.stream_devices[0]));
}

MediaCaptureDevicesDispatcher *MediaCaptureDevicesDispatcher::GetInstance()
{
    return base::Singleton<MediaCaptureDevicesDispatcher>::get();
}

MediaCaptureDevicesDispatcher::MediaCaptureDevicesDispatcher()
    : m_webContentsCollection(this)
{
#if defined(Q_OS_WIN)
    // Currently loopback audio capture is supported only on Windows.
    m_loopbackAudioSupported = true;
#endif
}

MediaCaptureDevicesDispatcher::~MediaCaptureDevicesDispatcher()
{
}

void MediaCaptureDevicesDispatcher::WebContentsDestroyed(content::WebContents *webContents)
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    m_pendingRequests.erase(webContents);
}

void MediaCaptureDevicesDispatcher::processMediaAccessRequest(content::WebContents *webContents, const content::MediaStreamRequest &request, content::MediaResponseCallback callback)
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    // Ensure we are observing the deletion of |webContents|.
    m_webContentsCollection.StartObserving(webContents);

    WebContentsAdapterClient::MediaRequestFlags flags = mediaRequestFlagsForRequest(request);
    if (!flags) {
        std::move(callback).Run(blink::mojom::StreamDevicesSet(), MediaStreamRequestResult::NOT_SUPPORTED, std::unique_ptr<content::MediaStreamUI>());
        return;
    }

    WebContentsDelegateQt *delegate = static_cast<WebContentsDelegateQt *>(webContents->GetDelegate());
    WebContentsAdapterClient *adapterClient = delegate->adapterClient();

    if (flags.testFlag(WebContentsAdapterClient::MediaDesktopVideoCapture)) {
        const bool screenCaptureEnabled = adapterClient->webEngineSettings()->testAttribute(
                QWebEngineSettings::ScreenCaptureEnabled);
        const bool originIsSecure = network::IsUrlPotentiallyTrustworthy(request.security_origin);
        if (!screenCaptureEnabled || !originIsSecure) {
            std::move(callback).Run(blink::mojom::StreamDevicesSet(), MediaStreamRequestResult::INVALID_STATE, std::unique_ptr<content::MediaStreamUI>());
            return;
        }

        if (!request.requested_video_device_id.empty()) {
            // Non-empty device id from the chooseDesktopMedia() extension API.
            processDesktopCaptureAccessRequest(webContents, request, std::move(callback));
            return;
        }
    }

    enqueueMediaAccessRequest(webContents, request, std::move(callback));
    // We might not require this approval for pepper requests.
    adapterClient->runMediaAccessPermissionRequest(toQt(request.security_origin), flags);
}

void MediaCaptureDevicesDispatcher::processDesktopCaptureAccessRequest(content::WebContents *webContents, const content::MediaStreamRequest &request, content::MediaResponseCallback callback)
{
    blink::mojom::StreamDevicesSet deviceSet;

    content::WebContents *const web_contents_for_stream = content::WebContents::FromRenderFrameHost(
            content::RenderFrameHost::FromID(request.render_process_id, request.render_frame_id));
    content::RenderFrameHost *const main_frame = web_contents_for_stream ? web_contents_for_stream->GetPrimaryMainFrame() : nullptr;

    content::DesktopMediaID mediaId;
    if (main_frame) {
        // The extension name that the stream is registered with.
        std::string originalExtensionName;
        // Resolve DesktopMediaID for the specified device id.
        mediaId = content::DesktopStreamsRegistry::GetInstance()->RequestMediaForStreamId(
                request.requested_video_device_id, main_frame->GetProcess()->GetID(),
                main_frame->GetRoutingID(), url::Origin::Create(request.security_origin),
                &originalExtensionName, content::kRegistryStreamTypeDesktop);
    }

    // Received invalid device id.
    if (mediaId.type == content::DesktopMediaID::TYPE_NONE) {
        std::move(callback).Run(deviceSet, MediaStreamRequestResult::INVALID_STATE, std::unique_ptr<content::MediaStreamUI>());
        return;
    }

    // Audio is only supported for screen capture streams.
    bool audioRequested = (request.audio_type == MediaStreamType::GUM_DESKTOP_AUDIO_CAPTURE ||
                           request.audio_type == MediaStreamType::DISPLAY_AUDIO_CAPTURE);
    bool audioSupported = (mediaId.type == content::DesktopMediaID::TYPE_SCREEN && m_loopbackAudioSupported);
    bool captureAudio = (audioRequested && audioSupported);

    deviceSet.stream_devices.emplace_back(blink::mojom::StreamDevices::New());
    blink::mojom::StreamDevices &stream_devices = *deviceSet.stream_devices[0];
    getDevicesForDesktopCapture(request, webContents, mediaId, captureAudio, request.disable_local_echo, stream_devices);

    if (deviceSet.stream_devices.empty())
        std::move(callback).Run(deviceSet, MediaStreamRequestResult::INVALID_STATE,
                                std::unique_ptr<content::MediaStreamUI>());
    else
        std::move(callback).Run(deviceSet, MediaStreamRequestResult::OK,
                                std::make_unique<MediaStreamUIQt>(webContents, *deviceSet.stream_devices[0]));
}

void MediaCaptureDevicesDispatcher::enqueueMediaAccessRequest(content::WebContents *webContents,
                                                              const content::MediaStreamRequest &request,
                                                              content::MediaResponseCallback callback)
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);

    RequestsQueue &queue = m_pendingRequests[webContents];
    queue.push_back(std::make_unique<PendingAccessRequest>(request, std::move(callback)));
}

void MediaCaptureDevicesDispatcher::ProcessQueuedAccessRequest(content::WebContents *webContents)
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);

    auto it = m_pendingRequests.find(webContents);
    if (it == m_pendingRequests.end() || it->second.empty())
        return;

    RequestsQueue &queue(it->second);
    content::MediaStreamRequest &request = queue.front()->request;

    WebContentsAdapterClient *adapterClient = WebContentsViewQt::from(static_cast<content::WebContentsImpl *>(webContents)->GetView())->client();
    adapterClient->runMediaAccessPermissionRequest(toQt(request.security_origin), mediaRequestFlagsForRequest(request));
}

void MediaCaptureDevicesDispatcher::getDefaultDevices(const std::string &audioDeviceId, const std::string &videoDeviceId,
                                                      bool audio, bool video, blink::mojom::StreamDevicesSet &devicesSet)
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);
    DCHECK(audio || video);

    devicesSet.stream_devices.emplace_back(blink::mojom::StreamDevices::New());
    blink::mojom::StreamDevices& devices = *devicesSet.stream_devices[0];
    if (audio) {
        const blink::MediaStreamDevices &audioDevices = content::MediaCaptureDevices::GetInstance()->GetAudioCaptureDevices();
        const blink::MediaStreamDevice *device = findDeviceWithId(audioDevices, audioDeviceId);
        if (!device && !audioDevices.empty())
            device = &audioDevices.front();
        if (device)
            devices.audio_device = *device;
    }

    if (video) {
        const blink::MediaStreamDevices &videoDevices = content::MediaCaptureDevices::GetInstance()->GetVideoCaptureDevices();
        const blink::MediaStreamDevice *device = findDeviceWithId(videoDevices, videoDeviceId);
        if (!device && !videoDevices.empty())
            device = &videoDevices.front();
        if (device)
            devices.video_device = *device;
    }
}

void MediaCaptureDevicesDispatcher::OnMediaRequestStateChanged(int render_process_id, int render_frame_id, int page_request_id, const GURL &security_origin, blink::mojom::MediaStreamType stream_type, content::MediaRequestState state)
{
    DCHECK_CURRENTLY_ON(BrowserThread::IO);
    content::GetUIThreadTaskRunner({})->PostTask(FROM_HERE,
                   base::BindOnce(&MediaCaptureDevicesDispatcher::updateMediaRequestStateOnUIThread,
                                  base::Unretained(this), render_process_id, render_frame_id,
                                  page_request_id, security_origin, stream_type, state));
}

void MediaCaptureDevicesDispatcher::updateMediaRequestStateOnUIThread(int render_process_id,
                                                                      int render_frame_id,
                                                                      int page_request_id,
                                                                      const GURL & /*security_origin*/,
                                                                      blink::mojom::MediaStreamType /*stream_type*/,
                                                                      content::MediaRequestState state)
{
    DCHECK_CURRENTLY_ON(BrowserThread::UI);

    // Cancel the request.
    if (state == content::MEDIA_REQUEST_STATE_CLOSING) {
        for (auto &pair : m_pendingRequests) {
            RequestsQueue &queue = pair.second;
            for (auto it = queue.begin(); it != queue.end(); ++it) {
                if ((*it)->request.render_process_id == render_process_id
                        && (*it)->request.render_frame_id == render_frame_id
                        && (*it)->request.page_request_id == page_request_id) {
                    queue.erase(it);
                    return;
                }
            }
        }
    }
}

} // namespace QtWebEngineCore
