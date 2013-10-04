/****************************************************************************
**
** Copyright 2013 The Chromium Authors. All rights reserved.
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

#ifndef DESKTOP_STREAMS_REGISTRY_H_
#define DESKTOP_STREAMS_REGISTRY_H_

#include <map>
#include <string>

#include "desktop_media_list.h"
#include "url/gurl.h"

// DesktopStreamsRegistry is used to store accepted desktop media streams for
// Desktop Capture API. Single instance of this class is created per browser in
// MediaCaptureDevicesDispatcher.
class DesktopStreamsRegistry {
 public:
  DesktopStreamsRegistry();
  ~DesktopStreamsRegistry();

  // Adds new stream to the registry. Called by the implementation of
  // desktopCapture.chooseDesktopMedia() API after user has approved access to
  // |source| for the |origin|. Returns identifier of the new stream.
  std::string RegisterStream(int render_process_id,
                             int render_view_id,
                             const GURL& origin,
                             const content::DesktopMediaID& source);

  // Validates stream identifier specified in getUserMedia(). Returns null
  // DesktopMediaID if the specified |id| is invalid, i.e. wasn't generated
  // using RegisterStream() or if it was generated for a different origin.
  // Otherwise returns ID of the source and removes it from the registry.
  content::DesktopMediaID RequestMediaForStreamId(const std::string& id,
                                                  int render_process_id,
                                                  int render_view_id,
                                                  const GURL& origin);

 private:
  // Type used to store list of accepted desktop media streams.
  struct ApprovedDesktopMediaStream {
    int render_process_id;
    int render_view_id;
    GURL origin;
    content::DesktopMediaID source;
  };
  typedef std::map<std::string, ApprovedDesktopMediaStream> StreamsMap;

  // Helper function that removes an expired stream from the registry.
  void CleanupStream(const std::string& id);

  StreamsMap approved_streams_;

  DISALLOW_COPY_AND_ASSIGN(DesktopStreamsRegistry);
};

#endif  // DESKTOP_STREAMS_REGISTRY_H_
