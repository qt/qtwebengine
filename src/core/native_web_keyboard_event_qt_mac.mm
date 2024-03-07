// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

// This is a workaround to be able to include Qt headers without
// "redefinition of 'NSString' as different kind of symbol" errors.
// TODO: Remove this when namespace ambiguity issues are fixed properly,
// see get_forward_declaration_macro() in cmake/Functions.cmake
#undef Q_FORWARD_DECLARE_OBJC_CLASS

#include "native_web_keyboard_event_qt.h"

#include <AppKit/AppKit.h>

#include "base/apple/owned_objc.h"

#include <QtGui/QKeyEvent>
#include <QtGui/private/qapplekeymapper_p.h>

namespace QtWebEngineCore {

base::apple::OwnedNSEvent ToNativeEvent(QKeyEvent *keyEvent)
{
    NSEventType type;
    switch (keyEvent->type()) {
    case QEvent::KeyPress:
        type = NSEventTypeKeyDown;
        break;
    case QEvent::KeyRelease:
        type = NSEventTypeKeyUp;
        break;
    default:
        Q_UNREACHABLE();
        return base::apple::OwnedNSEvent();
    }

    NSString *text = keyEvent->text().toNSString();
    if (text.length == 0) {
        Qt::Key key = static_cast<Qt::Key>(keyEvent->key());
        QChar cocoaKey = QAppleKeyMapper::toCocoaKey(key);
        text = QStringView(&cocoaKey, 1).toNSString();
    }

    return base::apple::OwnedNSEvent([NSEvent
                       keyEventWithType:type
                               location:NSZeroPoint
                          modifierFlags:QAppleKeyMapper::toCocoaModifiers(keyEvent->modifiers())
                              timestamp:keyEvent->timestamp() / 1000
                           windowNumber:0
                                context:nil
                             characters:text
            charactersIgnoringModifiers:text
                              isARepeat:keyEvent->isAutoRepeat()
                                keyCode:keyEvent->nativeVirtualKey()
    ]);
}

// Based on qtbase/src/plugins/platforms/cocoa/qnsview_keys.mm (KeyEvent::KeyEvent())
QKeyEvent *ToKeyEvent(base::apple::OwnedNSEvent event)
{
    NSEvent *nsevent = event.Get();

    QEvent::Type type = QEvent::None;
    switch (nsevent.type) {
    case NSEventTypeKeyDown:
        type = QEvent::KeyPress;
        break;
    case NSEventTypeKeyUp:
        type = QEvent::KeyRelease;
        break;
    default:
        Q_UNREACHABLE();
        return nullptr;
    }

    // Scan codes are hardware dependent codes for each key. There is no way to get these
    // from Carbon or Cocoa, so leave it 0, as documented in QKeyEvent::nativeScanCode().
    quint32 nativeScanCode = 0;
    quint32 nativeVirtualKey = nsevent.keyCode;

    NSEventModifierFlags nativeModifiers = nsevent.modifierFlags;
    Qt::KeyboardModifiers modifiers = QAppleKeyMapper::fromCocoaModifiers(nativeModifiers);

    NSString *charactersIgnoringModifiers = nsevent.charactersIgnoringModifiers;
    NSString *characters = nsevent.characters;

    // If a dead key occurs as a result of pressing a key combination then
    // characters will have 0 length, but charactersIgnoringModifiers will
    // have a valid character in it. This enables key combinations such as
    // ALT+E to be used as a shortcut with an English keyboard even though
    // pressing ALT+E will give a dead key while doing normal text input.
    Qt::Key key = Qt::Key_unknown;
    if (characters.length || charactersIgnoringModifiers.length) {
        QChar character = QChar::ReplacementCharacter;
        if (nativeModifiers & (NSEventModifierFlagControl | NSEventModifierFlagOption)
            && charactersIgnoringModifiers.length)
            character = QChar([charactersIgnoringModifiers characterAtIndex:0]);
        else if (characters.length)
            character = QChar([characters characterAtIndex:0]);
        key = QAppleKeyMapper::fromCocoaKey(character);
    }

    QString text = QString::fromNSString(characters);
    bool autorep = nsevent.ARepeat;

    return new QKeyEvent(type, key, modifiers, nativeScanCode, nativeVirtualKey, nativeModifiers,
                         text, autorep);
}

} // namespace QtWebEngineCore

namespace content {

NativeWebKeyboardEvent::NativeWebKeyboardEvent(const blink::WebKeyboardEvent &web_event, gfx::NativeView)
    : blink::WebKeyboardEvent(web_event)
    , skip_if_unhandled(false)
{
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(blink::WebInputEvent::Type type, int modifiers,
                                               base::TimeTicks timestamp)
    : blink::WebKeyboardEvent(type, modifiers, timestamp)
    , skip_if_unhandled(false)
{
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(gfx::NativeEvent native_event)
    : os_event(native_event) // FIXME: Copy?
    , skip_if_unhandled(false)
{
}

NativeWebKeyboardEvent::NativeWebKeyboardEvent(const NativeWebKeyboardEvent& other)
    : blink::WebKeyboardEvent(other)
    , os_event(other.os_event) // FIXME: Copy?
    , skip_if_unhandled(other.skip_if_unhandled)
{
}

NativeWebKeyboardEvent &NativeWebKeyboardEvent::operator=(const NativeWebKeyboardEvent &other)
{
    blink::WebKeyboardEvent::operator=(other);

    os_event = other.os_event; // FIXME: Copy?
    skip_if_unhandled = other.skip_if_unhandled;

    return *this;
}

NativeWebKeyboardEvent::~NativeWebKeyboardEvent() = default;

}  // namespace content
