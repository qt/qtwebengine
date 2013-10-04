/****************************************************************************
**
** Copyright (c) 2012 The Chromium Authors. All rights reserved.
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

#ifndef MEDIA_CAPTURE_DEVICES_DISPATCHER_H
#define MEDIA_CAPTURE_DEVICES_DISPATCHER_H

#include <deque>
#include <list>
#include <map>
#include <QtCore/qcompilerdetection.h>

#include "web_contents_adapter_client.h"

#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/singleton.h"
#include "base/observer_list.h"
#include "content/public/browser/media_observer.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/common/media_stream_request.h"

class AudioStreamIndicator;
class DesktopStreamsRegistry;
class MediaStreamCaptureIndicator;

// This singleton is used to receive updates about media events from the content
// layer. Based on Chrome's implementation.
class MediaCaptureDevicesDispatcher : public content::MediaObserver,
                                      public content::NotificationObserver {
 public:

  static MediaCaptureDevicesDispatcher* GetInstance();

  void processMediaAccessRequest(WebContentsAdapterClient *, content::WebContents *, const content::MediaStreamRequest &, const content::MediaResponseCallback &);

  // Called back from our WebContentsAdapter to grant the requested permission.
  void handleMediaAccessPermissionResponse(content::WebContents* , WebContentsAdapterClient::MediaRequestFlags);


  void GetDefaultDevices(const std::string & audioDeviceId, const std::string & videoDeviceId, bool audio, bool video, content::MediaStreamDevices*);

  const content::MediaStreamDevice* GetRequestedAudioDevice(const std::string& requested_audio_device_id);
  const content::MediaStreamDevice* GetRequestedVideoDevice(const std::string& requested_video_device_id);

  // Returns the first available audio or video device, or NULL if no devices
  // are available.
  const content::MediaStreamDevice* GetFirstAvailableAudioDevice();
  const content::MediaStreamDevice* GetFirstAvailableVideoDevice();

  // Overridden from content::MediaObserver:
  virtual void OnAudioCaptureDevicesChanged(const content::MediaStreamDevices& devices) Q_DECL_OVERRIDE;
  virtual void OnVideoCaptureDevicesChanged(const content::MediaStreamDevices& devices) Q_DECL_OVERRIDE;
  virtual void OnMediaRequestStateChanged(int render_process_id, int render_view_id, int page_request_id, const content::MediaStreamDevice& device
                                          , content::MediaRequestState state) Q_DECL_OVERRIDE;
  virtual void OnAudioStreamPlayingChanged(int /*render_process_id*/, int /*render_view_id*/, int /*stream_id*/,
                                           bool /*is_playing*/, float /*power_dBFS*/, bool /*clipped*/) Q_DECL_OVERRIDE {}
  virtual void OnCreatingAudioStream(int render_process_id, int render_view_id) Q_DECL_OVERRIDE;

  DesktopStreamsRegistry* GetDesktopStreamsRegistry();

  bool IsDesktopCaptureInProgress();

 private:
  friend struct DefaultSingletonTraits<MediaCaptureDevicesDispatcher>;

  struct PendingAccessRequest {
    PendingAccessRequest(const content::MediaStreamRequest& request,
                         const content::MediaResponseCallback& callback);
    ~PendingAccessRequest();

    content::MediaStreamRequest request;
    content::MediaResponseCallback callback;
  };
  typedef std::deque<PendingAccessRequest> RequestsQueue;
  typedef std::map<content::WebContents*, RequestsQueue> RequestsQueues;

  MediaCaptureDevicesDispatcher();
  virtual ~MediaCaptureDevicesDispatcher();

  const content::MediaStreamDevices &GetAudioCaptureDevices();
  const content::MediaStreamDevices &GetVideoCaptureDevices();

  // content::NotificationObserver implementation.
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) Q_DECL_OVERRIDE;

  // Helpers for ProcessMediaAccessRequest().
  void ProcessDesktopCaptureAccessRequest(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback);
  void ProcessScreenCaptureAccessRequest(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback);
  void ProcessRegularMediaAccessRequest(
      content::WebContents* web_contents,
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback);
  void ProcessQueuedAccessRequest(content::WebContents* web_contents);
  void OnAccessRequestResponse(content::WebContents* web_contents,
                               const content::MediaStreamDevices& devices,
                               scoped_ptr<content::MediaStreamUI> ui);

  // Called by the MediaObserver() functions, executed on UI thread.
  void UpdateAudioDevicesOnUIThread(const content::MediaStreamDevices& devices);
  void UpdateVideoDevicesOnUIThread(const content::MediaStreamDevices& devices);
  void UpdateMediaRequestStateOnUIThread(
      int render_process_id,
      int render_view_id,
      int page_request_id,
      const content::MediaStreamDevice& device,
      content::MediaRequestState state);
  void OnCreatingAudioStreamOnUIThread(int render_process_id,
                                       int render_view_id);

  // A list of cached audio capture devices.
  content::MediaStreamDevices audio_devices_;

  // A list of cached video capture devices.
  content::MediaStreamDevices video_devices_;

  // Flag to indicate if device enumeration has been done/doing.
  // Only accessed on UI thread.
  bool devices_enumerated_;

  RequestsQueues pending_requests_;

  scoped_ptr<DesktopStreamsRegistry> desktop_streams_registry_;

  content::NotificationRegistrar notifications_registrar_;

  // Tracks MEDIA_DESKTOP_VIDEO_CAPTURE sessions which reach the
  // MEDIA_REQUEST_STATE_DONE state.  Sessions are remove when
  // MEDIA_REQUEST_STATE_CLOSING is encountered.
  struct DesktopCaptureSession {
    int render_process_id;
    int render_view_id;
    int page_request_id;
  };
  typedef std::list<DesktopCaptureSession> DesktopCaptureSessions;
  DesktopCaptureSessions desktop_capture_sessions_;

  DISALLOW_COPY_AND_ASSIGN(MediaCaptureDevicesDispatcher);
};

#endif  // MEDIA_CAPTURE_DEVICES_DISPATCHER_H
