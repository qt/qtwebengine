// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media_capture_devices_dispatcher.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/media_devices_monitor.h"
#include "content/public/common/media_stream_request.h"

using content::BrowserThread;
using content::MediaStreamDevices;

namespace {

const content::MediaStreamDevice* FindDefaultDeviceWithId(
    const content::MediaStreamDevices& devices,
    const std::string& device_id) {
  if (devices.empty()) {
    return NULL;
  }
  content::MediaStreamDevices::const_iterator iter = devices.begin();
  for (; iter != devices.end(); ++iter) {
    if (iter->id == device_id) {
      return &(*iter);
    }
  }

  return &(*devices.begin());
};

}  // namespace


MediaCaptureDevicesDispatcher*
    MediaCaptureDevicesDispatcher::GetInstance() {
  return Singleton<MediaCaptureDevicesDispatcher>::get();
}

void MediaCaptureDevicesDispatcher::RunRequestMediaAccessPermission(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {

  content::MediaStreamDevices devices;
  // Based on chrome/browser/media/media_stream_devices_controller.cc.
  bool microphone_requested =
      (request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE);
  bool webcam_requested =
      (request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE);
  if (microphone_requested || webcam_requested) {
    switch (request.request_type) {
      case content::MEDIA_OPEN_DEVICE:
        // For open device request pick the desired device or fall back to the
        // first available of the given type.
        MediaCaptureDevicesDispatcher::GetInstance()->GetRequestedDevice(
            request.requested_audio_device_id,
            request.requested_video_device_id,
            microphone_requested,
            webcam_requested,
            &devices);
        break;
      case content::MEDIA_DEVICE_ACCESS:
      case content::MEDIA_GENERATE_STREAM:
      case content::MEDIA_ENUMERATE_DEVICES:
        // Get the default devices for the request.
        MediaCaptureDevicesDispatcher::GetInstance()->GetRequestedDevice(
            "", "",
            microphone_requested,
            webcam_requested,
            &devices);
        break;
    }
  }
  callback.Run(devices, scoped_ptr<content::MediaStreamUI>());
}

MediaCaptureDevicesDispatcher::MediaCaptureDevicesDispatcher()
    : devices_enumerated_(false) {}

MediaCaptureDevicesDispatcher::~MediaCaptureDevicesDispatcher() {}

void MediaCaptureDevicesDispatcher::AddObserver(Observer* observer) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (!observers_.HasObserver(observer))
    observers_.AddObserver(observer);
}

void MediaCaptureDevicesDispatcher::RemoveObserver(Observer* observer) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  observers_.RemoveObserver(observer);
}

const MediaStreamDevices&
MediaCaptureDevicesDispatcher::GetAudioCaptureDevices() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (!devices_enumerated_) {
    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(&content::EnsureMonitorCaptureDevices));
    devices_enumerated_ = true;
  }
  return audio_devices_;
}

const MediaStreamDevices&
MediaCaptureDevicesDispatcher::GetVideoCaptureDevices() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (!devices_enumerated_) {
    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(&content::EnsureMonitorCaptureDevices));
    devices_enumerated_ = true;
  }
  return video_devices_;
}

void MediaCaptureDevicesDispatcher::GetRequestedDevice(
    const std::string& requested_audio_device_id,
    const std::string& requested_video_device_id,
    bool audio,
    bool video,
    content::MediaStreamDevices* devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  DCHECK(audio || video);

  if (audio) {
    const content::MediaStreamDevices& audio_devices = GetAudioCaptureDevices();
    const content::MediaStreamDevice* const device =
        FindDefaultDeviceWithId(audio_devices, requested_audio_device_id);
    if (device)
      devices->push_back(*device);
  }
  if (video) {
    const content::MediaStreamDevices& video_devices = GetVideoCaptureDevices();
    const content::MediaStreamDevice* const device =
        FindDefaultDeviceWithId(video_devices, requested_video_device_id);
    if (device)
      devices->push_back(*device);
  }
}

void MediaCaptureDevicesDispatcher::OnAudioCaptureDevicesChanged(
    const content::MediaStreamDevices& devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(
          &MediaCaptureDevicesDispatcher::UpdateAudioDevicesOnUIThread,
          base::Unretained(this), devices));
}

void MediaCaptureDevicesDispatcher::OnVideoCaptureDevicesChanged(
    const content::MediaStreamDevices& devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(
          &MediaCaptureDevicesDispatcher::UpdateVideoDevicesOnUIThread,
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
          &MediaCaptureDevicesDispatcher::UpdateMediaReqStateOnUIThread,
          base::Unretained(this), render_process_id, render_view_id, device,
          state));
}

void MediaCaptureDevicesDispatcher::OnAudioStreamPlayingChanged(
    int render_process_id, int render_view_id, int stream_id, bool playing, float, bool) {
}

void MediaCaptureDevicesDispatcher::UpdateAudioDevicesOnUIThread(
    const content::MediaStreamDevices& devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  devices_enumerated_ = true;
  audio_devices_ = devices;
  FOR_EACH_OBSERVER(Observer, observers_,
                    OnUpdateAudioDevices(audio_devices_));
}

void MediaCaptureDevicesDispatcher::UpdateVideoDevicesOnUIThread(
    const content::MediaStreamDevices& devices) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  devices_enumerated_ = true;
  video_devices_ = devices;
  FOR_EACH_OBSERVER(Observer, observers_,
                    OnUpdateVideoDevices(video_devices_));
}

void MediaCaptureDevicesDispatcher::UpdateMediaReqStateOnUIThread(
    int render_process_id,
    int render_view_id,
    const content::MediaStreamDevice& device,
    content::MediaRequestState state) {
  FOR_EACH_OBSERVER(Observer, observers_,
                    OnRequestUpdate(render_process_id,
                                    render_view_id,
                                    device,
                                    state));
}
