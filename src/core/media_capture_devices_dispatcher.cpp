/****************************************************************************
**
** Copyright 2012 The Chromium Authors. All rights reserved.
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

#include "media_capture_devices_dispatcher.h"

#include "desktop_streams_registry.h"
#include "type_conversion.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/sha1.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/desktop_media_id.h"
#include "content/public/browser/media_devices_monitor.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/media_stream_request.h"
#include "media/audio/audio_manager_base.h"
#include "ui/base/l10n/l10n_util.h"

using content::BrowserThread;
using content::MediaStreamDevices;

namespace {

// Finds a device in |devices| that has |device_id|, or NULL if not found.
const content::MediaStreamDevice* FindDeviceWithId(const content::MediaStreamDevices& devices, const std::string& device_id)
{
  content::MediaStreamDevices::const_iterator iter = devices.begin();
  for (; iter != devices.end(); ++iter) {
    if (iter->id == device_id) {
      return &(*iter);
    }
  }
  return NULL;
}

// Helper to get title of the calling application shown in the screen capture
// notification.
base::string16 GetApplicationTitle(content::WebContents* web_contents) {
  return UTF8ToUTF16(web_contents->GetURL().GetOrigin().spec());
}

// Helper to get list of media stream devices for desktop capture in |devices|.
// Registers to display notification if |display_notification| is true.
// Returns an instance of MediaStreamUI to be passed to content layer.
scoped_ptr<content::MediaStreamUI> GetDevicesForDesktopCapture(
    content::MediaStreamDevices& devices,
    content::DesktopMediaID media_id,
    bool capture_audio,
    bool display_notification,
    base::string16 application_title) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  scoped_ptr<content::MediaStreamUI> ui;

  // Add selected desktop source to the list.
  devices.push_back(content::MediaStreamDevice(
      content::MEDIA_DESKTOP_VIDEO_CAPTURE, media_id.ToString(), "Screen"));
  if (capture_audio) {
    // Use the special loopback device ID for system audio capture.
    devices.push_back(content::MediaStreamDevice(
        content::MEDIA_LOOPBACK_AUDIO_CAPTURE,
        media::AudioManagerBase::kLoopbackInputDeviceId, "System Audio"));
  }

  // If required, register to display the notification for stream capture.
  // FIXME: implement a MediaStreamUI for this and expose a way to stop the capture ###
//  if (display_notification) {
//    ui = ScreenCaptureNotificationUI::Create(l10n_util::GetStringFUTF16(
//        IDS_MEDIA_SCREEN_CAPTURE_NOTIFICATION_TEXT,
//        application_title));
//  }

  return ui.Pass();
}

}  // namespace

void MediaCaptureDevicesDispatcher::handleMediaAccessPermissionResponse(content::WebContents *webContents, WebContentsAdapterClient::MediaRequestFlags authorizationFlags)
{
    content::MediaStreamDevices devices;
    std::map<content::WebContents*, RequestsQueue>::iterator it =
            pending_requests_.find(webContents);
    if (it == pending_requests_.end()) {
        // WebContents has been destroyed. Don't need to do anything.
        return;
    }

    RequestsQueue& queue(it->second);
    if (queue.empty())
        return;

    content::MediaStreamRequest& request = queue.front().request;

    bool microphone_requested =
        (request.audio_type && authorizationFlags & WebContentsAdapterClient::MediaAudioCapture);
    bool webcam_requested =
        (request.video_type && authorizationFlags & WebContentsAdapterClient::MediaVideoCapture);
    if (microphone_requested || webcam_requested) {
        switch (request.request_type) {
        case content::MEDIA_OPEN_DEVICE:
            Q_UNREACHABLE(); // only speculative as this is for Pepper
            GetDefaultDevices("", "", microphone_requested, webcam_requested, &devices);
            break;
        case content::MEDIA_DEVICE_ACCESS:
        case content::MEDIA_GENERATE_STREAM:
        case content::MEDIA_ENUMERATE_DEVICES:
            GetDefaultDevices(request.requested_audio_device_id, request.requested_video_device_id,
                        microphone_requested, webcam_requested, &devices);
            break;
        }
    }

    OnAccessRequestResponse(webContents, devices, scoped_ptr<content::MediaStreamUI>());
}


MediaCaptureDevicesDispatcher::PendingAccessRequest::PendingAccessRequest(
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback)
    : request(request),
      callback(callback) {
}

MediaCaptureDevicesDispatcher::PendingAccessRequest::~PendingAccessRequest() {}

MediaCaptureDevicesDispatcher* MediaCaptureDevicesDispatcher::GetInstance() {
  return Singleton<MediaCaptureDevicesDispatcher>::get();
}

MediaCaptureDevicesDispatcher::MediaCaptureDevicesDispatcher()
    : devices_enumerated_(false)
{
  // MediaCaptureDevicesDispatcher is a singleton. It should be created on
  // UI thread. Otherwise, it will not receive
  // content::NOTIFICATION_WEB_CONTENTS_DESTROYED, and that will result in
  // possible use after free.
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  notifications_registrar_.Add(this, content::NOTIFICATION_WEB_CONTENTS_DESTROYED,
                               content::NotificationService::AllSources());
}

MediaCaptureDevicesDispatcher::~MediaCaptureDevicesDispatcher() {}

const MediaStreamDevices&
MediaCaptureDevicesDispatcher::GetAudioCaptureDevices() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (!devices_enumerated_) {
    content::EnsureMonitorCaptureDevices();
    devices_enumerated_ = true;
  }
  return audio_devices_;
}

const MediaStreamDevices&
MediaCaptureDevicesDispatcher::GetVideoCaptureDevices() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (!devices_enumerated_) {
    content::EnsureMonitorCaptureDevices();
    devices_enumerated_ = true;
  }
  return video_devices_;
}

void MediaCaptureDevicesDispatcher::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (type == content::NOTIFICATION_WEB_CONTENTS_DESTROYED) {
    content::WebContents* web_contents =
        content::Source<content::WebContents>(source).ptr();
    pending_requests_.erase(web_contents);
  }
}

void MediaCaptureDevicesDispatcher::processMediaAccessRequest(WebContentsAdapterClient *adapterClient, content::WebContents* web_contents
                                                              , const content::MediaStreamRequest& request
                                                              , const content::MediaResponseCallback& callback)
{
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  // Let's not support tab capture for now.
  if (request.video_type == content::MEDIA_TAB_VIDEO_CAPTURE || request.audio_type == content::MEDIA_TAB_AUDIO_CAPTURE)
      return;

  if (request.video_type == content::MEDIA_DESKTOP_VIDEO_CAPTURE || request.audio_type == content::MEDIA_LOOPBACK_AUDIO_CAPTURE)
      ProcessDesktopCaptureAccessRequest(web_contents, request, callback);
  else {
      ProcessRegularMediaAccessRequest(web_contents, request, callback);
      WebContentsAdapterClient::MediaRequestFlags requestFlags;
      if (request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE)
          requestFlags |= WebContentsAdapterClient::MediaAudioCapture;
      if (request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE)
          requestFlags |= WebContentsAdapterClient::MediaVideoCapture;

      // We might not require this approval for pepper requests.
      adapterClient->runMediaAccessPermissionRequest(toQt(request.security_origin), requestFlags);

  }

}

void MediaCaptureDevicesDispatcher::ProcessDesktopCaptureAccessRequest(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  content::MediaStreamDevices devices;
  scoped_ptr<content::MediaStreamUI> ui;

  if (request.video_type != content::MEDIA_DESKTOP_VIDEO_CAPTURE) {
    callback.Run(devices, ui.Pass());
    return;
  }

  // If the device id wasn't specified then this is a screen capture request
  // (i.e. chooseDesktopMedia() API wasn't used to generate device id).
  if (request.requested_video_device_id.empty()) {
    ProcessScreenCaptureAccessRequest(
        web_contents, request, callback);
    return;
  }

  // Resolve DesktopMediaID for the specified device id.
  content::DesktopMediaID media_id =
      GetDesktopStreamsRegistry()->RequestMediaForStreamId(
          request.requested_video_device_id, request.render_process_id,
          request.render_view_id, request.security_origin);

  // Received invalid device id.
  if (media_id.type == content::DesktopMediaID::TYPE_NONE) {
    callback.Run(devices, ui.Pass());
    return;
  }

  bool loopback_audio_supported = false;
#if defined(USE_CRAS) || defined(OS_WIN)
  // Currently loopback audio capture is supported only on Windows and ChromeOS.
  loopback_audio_supported = true;
#endif

  // Audio is only supported for screen capture streams.
  bool capture_audio =
      (media_id.type == content::DesktopMediaID::TYPE_SCREEN &&
       request.audio_type == content::MEDIA_LOOPBACK_AUDIO_CAPTURE &&
       loopback_audio_supported);

  ui = GetDevicesForDesktopCapture(
      devices, media_id, capture_audio, true,
      GetApplicationTitle(web_contents));

  callback.Run(devices, ui.Pass());
}

void MediaCaptureDevicesDispatcher::ProcessScreenCaptureAccessRequest(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  content::MediaStreamDevices devices;
  scoped_ptr<content::MediaStreamUI> ui;

  DCHECK_EQ(request.video_type, content::MEDIA_DESKTOP_VIDEO_CAPTURE);

  bool loopback_audio_supported = false;
#if defined(USE_CRAS) || defined(OS_WIN)
  // Currently loopback audio capture is supported only on Windows and ChromeOS.
  loopback_audio_supported = true;
#endif

  // FIXME: expose in settings?
  const bool screen_capture_enabled = true;

  const bool origin_is_secure =
      request.security_origin.SchemeIsSecure();

  if (screen_capture_enabled && origin_is_secure) {
    // Get title of the calling application prior to showing the message box.
    // chrome::ShowMessageBox() starts a nested message loop which may allow
    // |web_contents| to be destroyed on the UI thread before the message box
    // is closed. See http://crbug.com/326690.
    base::string16 application_title =
        GetApplicationTitle(web_contents);
    web_contents = NULL;

    // FIXME: dektop capture processing
    if (true/*user_approved*/) {
      content::DesktopMediaID screen_id;
#if defined(OS_CHROMEOS)
      screen_id = content::DesktopMediaID::RegisterAuraWindow(
          ash::Shell::GetInstance()->GetPrimaryRootWindow());
#else  // defined(OS_CHROMEOS)
      screen_id =
          content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, 0);
#endif  // !defined(OS_CHROMEOS)

      bool capture_audio =
          (request.audio_type == content::MEDIA_LOOPBACK_AUDIO_CAPTURE &&
           loopback_audio_supported);

      ui = GetDevicesForDesktopCapture(devices, screen_id,  capture_audio,
                                       false/*display_notification*/, application_title);
    }
  }

  callback.Run(devices, ui.Pass());
}

void MediaCaptureDevicesDispatcher::ProcessRegularMediaAccessRequest(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  RequestsQueue& queue = pending_requests_[web_contents];
  queue.push_back(PendingAccessRequest(request, callback));

  // If this is the only request then show the infobar.
  if (queue.size() == 1)
    ProcessQueuedAccessRequest(web_contents);
}

void MediaCaptureDevicesDispatcher::ProcessQueuedAccessRequest(
    content::WebContents* web_contents) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  std::map<content::WebContents*, RequestsQueue>::iterator it =
      pending_requests_.find(web_contents);

  if (it == pending_requests_.end() || it->second.empty())
    return;

  DCHECK(!it->second.empty());

  // FIXME: Wrap request and pass on to API layer here instead, to avoid overlapping notification bars
//  MediaStreamInfoBarDelegate::Create(
//      web_contents, it->second.front().request,
//      base::Bind(&MediaCaptureDevicesDispatcher::OnAccessRequestResponse,
//                 base::Unretained(this), web_contents));

}

void MediaCaptureDevicesDispatcher::OnAccessRequestResponse(
    content::WebContents* web_contents,
    const content::MediaStreamDevices& devices,
    scoped_ptr<content::MediaStreamUI> ui) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  std::map<content::WebContents*, RequestsQueue>::iterator it =
      pending_requests_.find(web_contents);
  if (it == pending_requests_.end()) {
    // WebContents has been destroyed. Don't need to do anything.
    return;
  }

  RequestsQueue& queue(it->second);
  if (queue.empty())
    return;

  content::MediaResponseCallback callback = queue.front().callback;
  queue.pop_front();

  if (!queue.empty()) {
    // Post a task to process next queued request. It has to be done
    // asynchronously to make sure that calling infobar is not destroyed until
    // after this function returns.
    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        base::Bind(&MediaCaptureDevicesDispatcher::ProcessQueuedAccessRequest,
                   base::Unretained(this), web_contents));
  }

  callback.Run(devices, ui.Pass());
}

void MediaCaptureDevicesDispatcher::GetDefaultDevices(const std::string &audioDeviceId, const std::string &videoDeviceId, bool audio, bool video, content::MediaStreamDevices* devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  DCHECK(audio || video);

  std::string default_device;
  if (audio) {
    const content::MediaStreamDevice* device = GetRequestedAudioDevice(audioDeviceId);
    if (!device)
      device = GetFirstAvailableAudioDevice();
    if (device)
      devices->push_back(*device);
  }

  if (video) {
    const content::MediaStreamDevice* device = GetRequestedVideoDevice(videoDeviceId);
    if (!device)
      device = GetFirstAvailableVideoDevice();
    if (device)
      devices->push_back(*device);
  }
}

const content::MediaStreamDevice*
MediaCaptureDevicesDispatcher::GetRequestedAudioDevice(
    const std::string& requested_audio_device_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  const content::MediaStreamDevices& audio_devices = GetAudioCaptureDevices();
  const content::MediaStreamDevice* const device =
      FindDeviceWithId(audio_devices, requested_audio_device_id);
  return device;
}

const content::MediaStreamDevice*
MediaCaptureDevicesDispatcher::GetFirstAvailableAudioDevice() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  const content::MediaStreamDevices& audio_devices = GetAudioCaptureDevices();
  if (audio_devices.empty())
    return NULL;
  return &(*audio_devices.begin());
}

const content::MediaStreamDevice*
MediaCaptureDevicesDispatcher::GetRequestedVideoDevice(
    const std::string& requested_video_device_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  const content::MediaStreamDevices& video_devices = GetVideoCaptureDevices();
  const content::MediaStreamDevice* const device =
      FindDeviceWithId(video_devices, requested_video_device_id);
  return device;
}

const content::MediaStreamDevice*
MediaCaptureDevicesDispatcher::GetFirstAvailableVideoDevice() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  const content::MediaStreamDevices& video_devices = GetVideoCaptureDevices();
  if (video_devices.empty())
    return NULL;
  return &(*video_devices.begin());
}

DesktopStreamsRegistry*
MediaCaptureDevicesDispatcher::GetDesktopStreamsRegistry() {
  if (!desktop_streams_registry_)
    desktop_streams_registry_.reset(new DesktopStreamsRegistry());
  return desktop_streams_registry_.get();
}

void MediaCaptureDevicesDispatcher::OnAudioCaptureDevicesChanged(
    const content::MediaStreamDevices& devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&MediaCaptureDevicesDispatcher::UpdateAudioDevicesOnUIThread,
                 base::Unretained(this), devices));
}

void MediaCaptureDevicesDispatcher::OnVideoCaptureDevicesChanged(
    const content::MediaStreamDevices& devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&MediaCaptureDevicesDispatcher::UpdateVideoDevicesOnUIThread,
                 base::Unretained(this), devices));
}

void MediaCaptureDevicesDispatcher::OnMediaRequestStateChanged(
    int render_process_id,
    int render_view_id,
    int page_request_id,
    const content::MediaStreamDevice& device,
    content::MediaRequestState state) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(
          &MediaCaptureDevicesDispatcher::UpdateMediaRequestStateOnUIThread,
          base::Unretained(this), render_process_id, render_view_id,
          page_request_id, device, state));
}

void MediaCaptureDevicesDispatcher::OnCreatingAudioStream(
    int render_process_id,
    int render_view_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(
          &MediaCaptureDevicesDispatcher::OnCreatingAudioStreamOnUIThread,
          base::Unretained(this), render_process_id, render_view_id));
}

void MediaCaptureDevicesDispatcher::UpdateAudioDevicesOnUIThread(
    const content::MediaStreamDevices& devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  devices_enumerated_ = true;
  audio_devices_ = devices;
}

void MediaCaptureDevicesDispatcher::UpdateVideoDevicesOnUIThread(
    const content::MediaStreamDevices& devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  devices_enumerated_ = true;
  video_devices_ = devices;
}

void MediaCaptureDevicesDispatcher::UpdateMediaRequestStateOnUIThread(
    int render_process_id,
    int render_view_id,
    int page_request_id,
    const content::MediaStreamDevice& device,
    content::MediaRequestState state) {
  // Track desktop capture sessions.  Tracking is necessary to avoid unbalanced
  // session counts since not all requests will reach MEDIA_REQUEST_STATE_DONE,
  // but they will all reach MEDIA_REQUEST_STATE_CLOSING.
  if (device.type == content::MEDIA_DESKTOP_VIDEO_CAPTURE) {
    if (state == content::MEDIA_REQUEST_STATE_DONE) {
      DesktopCaptureSession session = { render_process_id, render_view_id,
                                        page_request_id };
      desktop_capture_sessions_.push_back(session);
    } else if (state == content::MEDIA_REQUEST_STATE_CLOSING) {
      for (DesktopCaptureSessions::iterator it =
               desktop_capture_sessions_.begin();
           it != desktop_capture_sessions_.end();
           ++it) {
        if (it->render_process_id == render_process_id &&
            it->render_view_id == render_view_id &&
            it->page_request_id == page_request_id) {
          desktop_capture_sessions_.erase(it);
          break;
        }
      }
    }
  }

  // Cancel the request.
  if (state == content::MEDIA_REQUEST_STATE_CLOSING) {
    bool found = false;
    for (RequestsQueues::iterator rqs_it = pending_requests_.begin();
         rqs_it != pending_requests_.end(); ++rqs_it) {
      RequestsQueue& queue = rqs_it->second;
      for (RequestsQueue::iterator it = queue.begin();
           it != queue.end(); ++it) {
        if (it->request.render_process_id == render_process_id &&
            it->request.render_view_id == render_view_id &&
            it->request.page_request_id == page_request_id) {
          queue.erase(it);
          found = true;
          break;
        }
      }
      if (found)
        break;
    }
  }
}

void MediaCaptureDevicesDispatcher::OnCreatingAudioStreamOnUIThread(
    int render_process_id,
    int render_view_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
}

bool MediaCaptureDevicesDispatcher::IsDesktopCaptureInProgress() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  return desktop_capture_sessions_.size() > 0;
}
