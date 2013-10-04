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

#include "desktop_streams_registry.h"

#include "base/base64.h"
#include "base/location.h"
#include "base/time/time.h"
#include "content/public/browser/browser_thread.h"
#include "crypto/random.h"

namespace {

const int kStreamIdLengthBytes = 16;

const int kApprovedStreamTimeToLiveSeconds = 10;

std::string GenerateRandomStreamId() {
  char buffer[kStreamIdLengthBytes];
  crypto::RandBytes(buffer, arraysize(buffer));
  std::string result;
  base::Base64Encode(base::StringPiece(buffer, arraysize(buffer)),
                     &result);
  return result;
}

}  // namespace

DesktopStreamsRegistry::DesktopStreamsRegistry() {}
DesktopStreamsRegistry::~DesktopStreamsRegistry() {}

std::string DesktopStreamsRegistry::RegisterStream(
    int render_process_id,
    int render_view_id,
    const GURL& origin,
    const content::DesktopMediaID& source) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  std::string id = GenerateRandomStreamId();
  ApprovedDesktopMediaStream& stream = approved_streams_[id];
  stream.render_process_id = render_process_id;
  stream.render_view_id = render_view_id;
  stream.origin = origin;
  stream.source = source;

  content::BrowserThread::PostDelayedTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&DesktopStreamsRegistry::CleanupStream,
                 base::Unretained(this), id),
      base::TimeDelta::FromSeconds(kApprovedStreamTimeToLiveSeconds));

  return id;
}

content::DesktopMediaID DesktopStreamsRegistry::RequestMediaForStreamId(
    const std::string& id,
    int render_process_id,
    int render_view_id,
    const GURL& origin) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  StreamsMap::iterator it = approved_streams_.find(id);

  // Verify that if there is a request with the specified ID it was created for
  // the same origin and the same renderer.
  if (it == approved_streams_.end() ||
      render_process_id != it->second.render_process_id ||
      render_view_id != it->second.render_view_id ||
      origin != it->second.origin) {
    return content::DesktopMediaID();
  }

  content::DesktopMediaID result = it->second.source;
  approved_streams_.erase(it);
  return result;
}

void DesktopStreamsRegistry::CleanupStream(const std::string& id) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  approved_streams_.erase(id);
}
