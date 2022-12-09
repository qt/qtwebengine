// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

self.addEventListener('push', event => {
    const data = event.data.json();
    self.registration.showNotification(data.title, { body : data.text });
});
