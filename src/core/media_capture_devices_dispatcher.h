// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef MEDIA_CAPTURE_DEVICES_DISPATCHER_H
#define MEDIA_CAPTURE_DEVICES_DISPATCHER_H

#include "web_contents_adapter_client.h"

#include "base/containers/circular_deque.h"
#include "base/containers/flat_map.h"
#include "base/memory/singleton.h"
#include "chrome/browser/tab_contents/web_contents_collection.h"
#include "content/public/browser/media_observer.h"
#include "content/public/browser/media_stream_request.h"

namespace QtWebEngineCore {

// This singleton is used to receive updates about media events from the content
// layer. Based on Chrome's implementation.
class MediaCaptureDevicesDispatcher : public content::MediaObserver,
                                      public WebContentsCollection::Observer
{
public:
    static MediaCaptureDevicesDispatcher *GetInstance();

    void processMediaAccessRequest(content::WebContents *, const content::MediaStreamRequest &, content::MediaResponseCallback);

    // Called back from our WebContentsAdapter to grant the requested permission.
    void handleMediaAccessPermissionResponse(content::WebContents *, const QUrl &securityOrigin, WebContentsAdapterClient::MediaRequestFlags);

private:
    void getDefaultDevices(const std::string &audioDeviceId, const std::string &videoDeviceId, bool audio, bool video, blink::mojom::StreamDevicesSet &devices);

    // Overridden from content::MediaObserver:
    void OnAudioCaptureDevicesChanged() override {}
    void OnVideoCaptureDevicesChanged() override {}
    void OnMediaRequestStateChanged(int render_process_id,
                                    int render_frame_id,
                                    int page_request_id,
                                    const GURL &security_origin,
                                    blink::mojom::MediaStreamType stream_type,
                                    content::MediaRequestState state) override;

    void OnCreatingAudioStream(int /*render_process_id*/, int /*render_frame_id*/) override {}
    void OnSetCapturingLinkSecured(int /*render_process_id*/,
                                   int /*render_frame_id*/,
                                   int /*page_request_id*/,
                                   blink::mojom::MediaStreamType /*stream_type*/,
                                   bool /*is_secure*/) override {}

    friend struct base::DefaultSingletonTraits<MediaCaptureDevicesDispatcher>;

    struct PendingAccessRequest {
        PendingAccessRequest(const content::MediaStreamRequest &request, content::MediaResponseCallback callback);
        ~PendingAccessRequest();

        content::MediaStreamRequest request;
        content::MediaResponseCallback callback;
    };
    typedef base::circular_deque<std::unique_ptr<PendingAccessRequest>> RequestsQueue;
    typedef base::flat_map<content::WebContents *, RequestsQueue> RequestsQueues;

    MediaCaptureDevicesDispatcher();
    virtual ~MediaCaptureDevicesDispatcher();

    // WebContentsCollection::Observer:
    void WebContentsDestroyed(content::WebContents *webContents) override;

    // Helpers for ProcessMediaAccessRequest().
    void processDesktopCaptureAccessRequest(content::WebContents *, const content::MediaStreamRequest &, content::MediaResponseCallback);
    void enqueueMediaAccessRequest(content::WebContents *, const content::MediaStreamRequest &, content::MediaResponseCallback);
    void ProcessQueuedAccessRequest(content::WebContents *);

    // Called by the MediaObserver() functions, executed on UI thread.
    void updateMediaRequestStateOnUIThread(int render_process_id, int render_frame_id, int page_request_id, const GURL &security_origin,
                                           blink::mojom::MediaStreamType stream_type, content::MediaRequestState state);

    RequestsQueues m_pendingRequests;

    WebContentsCollection m_webContentsCollection;

    bool m_loopbackAudioSupported = false;
};

} // namespace QtWebEngineCore

#endif // MEDIA_CAPTURE_DEVICES_DISPATCHER_H
