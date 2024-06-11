// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "web_event_factory.h"
#include "third_party/blink/renderer/platform/windows_keyboard_codes.h"
#include "ui/events/keycodes/dom/dom_code.h"
#include "ui/events/keycodes/dom/dom_key.h"
#include "ui/events/keycodes/dom/keycode_converter.h"
#include "ui/events/keycodes/keyboard_code_conversion.h"

#include "render_widget_host_view_qt_delegate.h"

#include <QtGui/private/qtgui-config_p.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStyleHints>
#if QT_CONFIG(tabletevent)
#include <QPointingDevice>
#endif
#include <QWheelEvent>

namespace QtWebEngineCore {

using namespace blink;

enum class KeyboardDriver { Unknown, Windows, Cocoa, Xkb, Evdev };

static KeyboardDriver keyboardDriverImpl()
{
    QString platformName = QGuiApplication::platformName();

    if (platformName == QLatin1String("windows"))
        return KeyboardDriver::Windows;

    if (platformName == QLatin1String("cocoa"))
        return KeyboardDriver::Cocoa;

    if (platformName == QLatin1String("xcb") || platformName == QLatin1String("wayland"))
        return KeyboardDriver::Xkb;

#if QT_CONFIG(libinput)
    // Based on QEglFSIntegration::createInputHandlers and QLibInputKeyboard::processKey.
    if (platformName == QLatin1String("eglfs") && !qEnvironmentVariableIntValue("QT_QPA_EGLFS_NO_LIBINPUT"))
        return KeyboardDriver::Xkb;
#endif

#if QT_CONFIG(evdev)
    // Based on QEglFSIntegration::createInputHandlers.
    if (platformName == QLatin1String("eglfs"))
        return KeyboardDriver::Evdev;
#endif

    return KeyboardDriver::Unknown;
}

static KeyboardDriver keyboardDriver()
{
    static KeyboardDriver cached = keyboardDriverImpl();
    return cached;
}

// Qt swaps the Control and Meta keys on macOS (unless the attribute
// AA_MacDontSwapCtrlAndMeta is set). To preserve compatibility with Chromium we
// want to unswap them when forwarding events. The following two functions,
// qtKeyForKeyEvent and qtModifiersForEvent, should be used for accessing the
// key() and modifiers() properties to ensure that the unswapping is done
// consistently.
static int qtKeyForKeyEvent(const QKeyEvent *ev)
{
    int key = ev->key();
    if (keyboardDriver() == KeyboardDriver::Cocoa && !qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)) {
        if (key == Qt::Key_Control)
            return Qt::Key_Meta;
        if (key == Qt::Key_Meta)
            return Qt::Key_Control;
    }
    return key;
}

// See above
static Qt::KeyboardModifiers qtModifiersForEvent(const QInputEvent *ev)
{
    Qt::KeyboardModifiers modifiers = ev->modifiers();
    if (keyboardDriver() == KeyboardDriver::Cocoa && !qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)) {
        bool controlModifier = modifiers.testFlag(Qt::ControlModifier);
        bool metaModifier = modifiers.testFlag(Qt::MetaModifier);
        modifiers.setFlag(Qt::ControlModifier, metaModifier);
        modifiers.setFlag(Qt::MetaModifier, controlModifier);
    }
    return modifiers;
}

// QKeyEvent::text() has some limits that we need to work around.
//
// On Linux, the Control modifier transformation is applied [1]. For example,
// pressing Ctrl+@ generates the text "\u0000". We would like "@" instead.
// Windows also translates some control key combinations into ASCII control
// characters [2].
//
// [1]: https://www.x.org/releases/current/doc/kbproto/xkbproto.html#Interpreting_the_Control_Modifier
// [2]: https://docs.microsoft.com/en-us/windows/win32/learnwin32/keyboard-input#character-messages
//
// On macOS, if the Control modifier is used, then no text is generated at all.
// We need some text.
//
// The workaround is to use QKeyEvent::key() instead, when needed. Contrary to
// the documentation, QKeyEvent::key() is not at all limited to the values
// listed in the Qt::Key enum: it can actually contain any Unicode codepoint.
// The only drawback is that letters are always upper cased.
static QString qtTextForKeyEvent(const QKeyEvent *ev, int qtKey, Qt::KeyboardModifiers qtModifiers)
{
    QString text = ev->text();

    if (keyboardDriver() == KeyboardDriver::Xkb && (qtModifiers & Qt::ControlModifier)) {
        text.clear();
    }

    // Keep text for Ctrl+Alt key combinations on Windows. It is an alternative for AltGr.
    if (keyboardDriver() == KeyboardDriver::Windows
            && (qtModifiers & Qt::ControlModifier) && !(qtModifiers & Qt::AltModifier)) {
        text.clear();
    }

    return text;
}

// The 'native key code' in Chromium refers to
//
//   - On Windows: the Windows OEM scancode.
//   - On macOS: the NSEvent's keyCode.
//   - On Linux: The XKB keycode.
static quint32 nativeKeyCodeForKeyEvent(const QKeyEvent *ev)
{
    // Ifdefs here should match <ui/events/keycodes/dom/keycode_converter.cc>,
    // since that is where the native key code is eventually used.
    //
    // Note that Xkb key codes are only supported under Linux (no BSDs,
    // Cygwin/X, etc). Also evdev key codes are *not* supported for the same
    // reason.
#if defined(Q_OS_WINDOWS)
    if (keyboardDriver() == KeyboardDriver::Windows) {
        // see GetScanCodeFromLParam in events_win_utils.cc:
        quint32 scancode = ev->nativeScanCode() & 0xff;
        if (ev->nativeScanCode() & 0x100)
            scancode |= 0xe000;
        return scancode;
    }
    return 0;
#elif defined(Q_OS_MACOS)
    return keyboardDriver() == KeyboardDriver::Cocoa ? ev->nativeVirtualKey() : 0;
#elif defined(Q_OS_LINUX)
    // Do not set native code to menu key if it was mapped to something else.
    if (ev->nativeScanCode() == 135 && ev->key() != Qt::Key_Menu)
        return 0;
    return keyboardDriver() == KeyboardDriver::Xkb ? ev->nativeScanCode() : 0;
#else
    return 0; // 0 means unknown, KeyboardEvent.code will be empty string.
#endif
}

static int windowsKeyCodeForQtKey(int qtKey, bool isKeypad)
{
    // Determine wheter the event comes from the keypad
    if (isKeypad) {
        switch (qtKey) {
        case Qt::Key_0:
            return VK_NUMPAD0; // (60) Numeric keypad 0 key
        case Qt::Key_1:
            return VK_NUMPAD1; // (61) Numeric keypad 1 key
        case Qt::Key_2:
            return  VK_NUMPAD2; // (62) Numeric keypad 2 key
        case Qt::Key_3:
            return VK_NUMPAD3; // (63) Numeric keypad 3 key
        case Qt::Key_4:
            return VK_NUMPAD4; // (64) Numeric keypad 4 key
        case Qt::Key_5:
            return VK_NUMPAD5; // (65) Numeric keypad 5 key
        case Qt::Key_6:
            return VK_NUMPAD6; // (66) Numeric keypad 6 key
        case Qt::Key_7:
            return VK_NUMPAD7; // (67) Numeric keypad 7 key
        case Qt::Key_8:
            return VK_NUMPAD8; // (68) Numeric keypad 8 key
        case Qt::Key_9:
            return VK_NUMPAD9; // (69) Numeric keypad 9 key
        case Qt::Key_Asterisk:
            return VK_MULTIPLY; // (6A) Multiply key
        case Qt::Key_Plus:
            return VK_ADD; // (6B) Add key
        case Qt::Key_Minus:
            return VK_SUBTRACT; // (6D) Subtract key
        case Qt::Key_Period:
            return VK_DECIMAL; // (6E) Decimal key
        case Qt::Key_Slash:
            return VK_DIVIDE; // (6F) Divide key
        case Qt::Key_Equal:
            return VK_OEM_PLUS; // (BB) Equal key
        case Qt::Key_PageUp:
            return VK_PRIOR; // (21) PAGE UP key
        case Qt::Key_PageDown:
            return VK_NEXT; // (22) PAGE DOWN key
        case Qt::Key_End:
            return VK_END; // (23) END key
        case Qt::Key_Home:
            return VK_HOME; // (24) HOME key
        case Qt::Key_Left:
            return VK_LEFT; // (25) LEFT ARROW key
        case Qt::Key_Up:
            return VK_UP; // (26) UP ARROW key
        case Qt::Key_Right:
            return VK_RIGHT; // (27) RIGHT ARROW key
        case Qt::Key_Down:
            return VK_DOWN; // (28) DOWN ARROW key
        case Qt::Key_Enter:
        case Qt::Key_Return:
            return VK_RETURN; // (0D) Return key
        case Qt::Key_Insert:
            return VK_INSERT; // (2D) INS key
        case Qt::Key_Delete:
            return VK_DELETE; // (2E) DEL key
        case Qt::Key_Shift:
            return VK_SHIFT; // (10) SHIFT key
        case Qt::Key_Control:
            return VK_CONTROL; // (11) CTRL key
        case Qt::Key_Alt:
            return VK_MENU; // (12) ALT key
        default:
            return 0;
        }

    } else {
        switch (qtKey) {
        case Qt::Key_Backspace:
            return VK_BACK; // (08) BACKSPACE key
        case Qt::Key_Backtab:
        case Qt::Key_Tab:
            return VK_TAB; // (09) TAB key
        case Qt::Key_Clear:
            return VK_CLEAR; // (0C) CLEAR key
        case Qt::Key_Enter:
        case Qt::Key_Return:
            return VK_RETURN; // (0D) Return key
        case Qt::Key_Shift:
            return VK_SHIFT; // (10) SHIFT key
        case Qt::Key_Control:
            return VK_CONTROL; // (11) CTRL key
        case Qt::Key_Alt:
            return VK_MENU; // (12) ALT key

        case Qt::Key_F1:
            return VK_F1;
        case Qt::Key_F2:
            return VK_F2;
        case Qt::Key_F3:
            return VK_F3;
        case Qt::Key_F4:
            return VK_F4;
        case Qt::Key_F5:
            return VK_F5;
        case Qt::Key_F6:
            return VK_F6;
        case Qt::Key_F7:
            return VK_F7;
        case Qt::Key_F8:
            return VK_F8;
        case Qt::Key_F9:
            return VK_F9;
        case Qt::Key_F10:
            return VK_F10;
        case Qt::Key_F11:
            return VK_F11;
        case Qt::Key_F12:
            return VK_F12;
        case Qt::Key_F13:
            return VK_F13;
        case Qt::Key_F14:
            return VK_F14;
        case Qt::Key_F15:
            return VK_F15;
        case Qt::Key_F16:
            return VK_F16;
        case Qt::Key_F17:
            return VK_F17;
        case Qt::Key_F18:
            return VK_F18;
        case Qt::Key_F19:
            return VK_F19;
        case Qt::Key_F20:
            return VK_F20;
        case Qt::Key_F21:
            return VK_F21;
        case Qt::Key_F22:
            return VK_F22;
        case Qt::Key_F23:
            return VK_F23;
        case Qt::Key_F24:
            return VK_F24;

        case Qt::Key_Pause:
            return VK_PAUSE; // (13) PAUSE key
        case Qt::Key_CapsLock:
            return VK_CAPITAL; // (14) CAPS LOCK key
        case Qt::Key_Kana_Lock:
        case Qt::Key_Kana_Shift:
            return VK_KANA; // (15) Input Method Editor (IME) Kana mode
        case Qt::Key_Hangul:
            return VK_HANGUL; // VK_HANGUL (15) IME Hangul mode
            // VK_JUNJA (17) IME Junja mode
            // VK_FINAL (18) IME final mode
        case Qt::Key_Hangul_Hanja:
            return VK_HANJA; // (19) IME Hanja mode
        case Qt::Key_Kanji:
            return VK_KANJI; // (19) IME Kanji mode
        case Qt::Key_Escape:
            return VK_ESCAPE; // (1B) ESC key
            // VK_CONVERT (1C) IME convert
            // VK_NONCONVERT (1D) IME nonconvert
            // VK_ACCEPT (1E) IME accept
            // VK_MODECHANGE (1F) IME mode change request
        case Qt::Key_Space:
            return VK_SPACE; // (20) SPACEBAR
        case Qt::Key_PageUp:
            return VK_PRIOR; // (21) PAGE UP key
        case Qt::Key_PageDown:
            return VK_NEXT; // (22) PAGE DOWN key
        case Qt::Key_End:
            return VK_END; // (23) END key
        case Qt::Key_Home:
            return VK_HOME; // (24) HOME key
        case Qt::Key_Left:
            return VK_LEFT; // (25) LEFT ARROW key
        case Qt::Key_Up:
            return VK_UP; // (26) UP ARROW key
        case Qt::Key_Right:
            return VK_RIGHT; // (27) RIGHT ARROW key
        case Qt::Key_Down:
            return VK_DOWN; // (28) DOWN ARROW key
        case Qt::Key_Select:
            return VK_SELECT; // (29) SELECT key
        case Qt::Key_Print:
            return VK_SNAPSHOT; // (2A) PRINT key
        case Qt::Key_Execute:
            return VK_EXECUTE; // (2B) EXECUTE key
        case Qt::Key_Insert:
            return VK_INSERT; // (2D) INS key
        case Qt::Key_Delete:
            return VK_DELETE; // (2E) DEL key
        case Qt::Key_Help:
            return VK_HELP; // (2F) HELP key
        case Qt::Key_0:
        case Qt::Key_ParenRight:
            return VK_0; // (30) 0) key
        case Qt::Key_1:
        case Qt::Key_Exclam:
            return VK_1; // (31) 1 ! key
        case Qt::Key_2:
        case Qt::Key_At:
            return VK_2; // (32) 2 & key
        case Qt::Key_3:
        case Qt::Key_NumberSign:
            return VK_3; // case '3': case '#';
        case Qt::Key_4:
        case Qt::Key_Dollar: // (34) 4 key '$';
            return VK_4;
        case Qt::Key_5:
        case Qt::Key_Percent:
            return VK_5; // (35) 5 key  '%'
        case Qt::Key_6:
        case Qt::Key_AsciiCircum:
            return VK_6; // (36) 6 key  '^'
        case Qt::Key_7:
        case Qt::Key_Ampersand:
            return VK_7; // (37) 7 key  case '&'
        case Qt::Key_8:
        case Qt::Key_Asterisk:
            return VK_8; // (38) 8 key  '*'
        case Qt::Key_9:
        case Qt::Key_ParenLeft:
            return VK_9; // (39) 9 key '('
        case Qt::Key_A:
            return VK_A; // (41) A key case 'a': case 'A': return 0x41;
        case Qt::Key_B:
            return VK_B; // (42) B key case 'b': case 'B': return 0x42;
        case Qt::Key_C:
            return VK_C; // (43) C key case 'c': case 'C': return 0x43;
        case Qt::Key_D:
            return VK_D; // (44) D key case 'd': case 'D': return 0x44;
        case Qt::Key_E:
            return VK_E; // (45) E key case 'e': case 'E': return 0x45;
        case Qt::Key_F:
            return VK_F; // (46) F key case 'f': case 'F': return 0x46;
        case Qt::Key_G:
            return VK_G; // (47) G key case 'g': case 'G': return 0x47;
        case Qt::Key_H:
            return VK_H; // (48) H key case 'h': case 'H': return 0x48;
        case Qt::Key_I:
            return VK_I; // (49) I key case 'i': case 'I': return 0x49;
        case Qt::Key_J:
            return VK_J; // (4A) J key case 'j': case 'J': return 0x4A;
        case Qt::Key_K:
            return VK_K; // (4B) K key case 'k': case 'K': return 0x4B;
        case Qt::Key_L:
            return VK_L; // (4C) L key case 'l': case 'L': return 0x4C;
        case Qt::Key_M:
            return VK_M; // (4D) M key case 'm': case 'M': return 0x4D;
        case Qt::Key_N:
            return VK_N; // (4E) N key case 'n': case 'N': return 0x4E;
        case Qt::Key_O:
            return VK_O; // (4F) O key case 'o': case 'O': return 0x4F;
        case Qt::Key_P:
            return VK_P; // (50) P key case 'p': case 'P': return 0x50;
        case Qt::Key_Q:
            return VK_Q; // (51) Q key case 'q': case 'Q': return 0x51;
        case Qt::Key_R:
            return VK_R; // (52) R key case 'r': case 'R': return 0x52;
        case Qt::Key_S:
            return VK_S; // (53) S key case 's': case 'S': return 0x53;
        case Qt::Key_T:
            return VK_T; // (54) T key case 't': case 'T': return 0x54;
        case Qt::Key_U:
            return VK_U; // (55) U key case 'u': case 'U': return 0x55;
        case Qt::Key_V:
            return VK_V; // (56) V key case 'v': case 'V': return 0x56;
        case Qt::Key_W:
            return VK_W; // (57) W key case 'w': case 'W': return 0x57;
        case Qt::Key_X:
            return VK_X; // (58) X key case 'x': case 'X': return 0x58;
        case Qt::Key_Y:
            return VK_Y; // (59) Y key case 'y': case 'Y': return 0x59;
        case Qt::Key_Z:
            return VK_Z; // (5A) Z key case 'z': case 'Z': return 0x5A;
        case Qt::Key_Meta:
            return VK_LWIN; // (5B) Left Windows key (Microsoft Natural keyboard)
            // case Qt::Key_Meta_R: FIXME: What to do here?
            //    return VK_RWIN; // (5C) Right Windows key (Natural keyboard)
        case Qt::Key_Menu: // (5D) Applications key (Natural keyboard)
            return VK_APPS;
            // VK_SLEEP (5F) Computer Sleep key
            // VK_SEPARATOR (6C) Separator key
            // VK_SUBTRACT (6D) Subtract key
            // VK_DECIMAL (6E) Decimal key
            // VK_DIVIDE (6F) Divide key
            // handled by key code above

        case Qt::Key_NumLock:
            return VK_NUMLOCK; // (90) NUM LOCK key

        case Qt::Key_ScrollLock:
            return VK_SCROLL; // (91) SCROLL LOCK key

            // VK_LSHIFT (A0) Left SHIFT key
            // VK_RSHIFT (A1) Right SHIFT key
            // VK_LCONTROL (A2) Left CONTROL key
            // VK_RCONTROL (A3) Right CONTROL key
            // VK_LMENU (A4) Left MENU key
            // VK_RMENU (A5) Right MENU key
            // VK_BROWSER_BACK (A6) Windows 2000/XP: Browser Back key
            // VK_BROWSER_FORWARD (A7) Windows 2000/XP: Browser Forward key
            // VK_BROWSER_REFRESH (A8) Windows 2000/XP: Browser Refresh key
            // VK_BROWSER_STOP (A9) Windows 2000/XP: Browser Stop key
            // VK_BROWSER_SEARCH (AA) Windows 2000/XP: Browser Search key
            // VK_BROWSER_FAVORITES (AB) Windows 2000/XP: Browser Favorites key
            // VK_BROWSER_HOME (AC) Windows 2000/XP: Browser Start and Home key

        case Qt::Key_VolumeMute:
            return VK_VOLUME_MUTE; // (AD) Windows 2000/XP: Volume Mute key
        case Qt::Key_VolumeDown:
            return VK_VOLUME_DOWN; // (AE) Windows 2000/XP: Volume Down key
        case Qt::Key_VolumeUp:
            return VK_VOLUME_UP; // (AF) Windows 2000/XP: Volume Up key
        case Qt::Key_MediaNext:
            return VK_MEDIA_NEXT_TRACK; // (B0) Windows 2000/XP: Next Track key
        case Qt::Key_MediaPrevious:
            return VK_MEDIA_PREV_TRACK; // (B1) Windows 2000/XP: Previous Track key
        case Qt::Key_MediaStop:
            return VK_MEDIA_STOP; // (B2) Windows 2000/XP: Stop Media key
        case Qt::Key_MediaTogglePlayPause:
            return VK_MEDIA_PLAY_PAUSE; // (B3) Windows 2000/XP: Play/Pause Media key

            // VK_LAUNCH_MAIL (B4) Windows 2000/XP: Start Mail key
            // VK_LAUNCH_MEDIA_SELECT (B5) Windows 2000/XP: Select Media key
            // VK_LAUNCH_APP1 (B6) Windows 2000/XP: Start Application 1 key
            // VK_LAUNCH_APP2 (B7) Windows 2000/XP: Start Application 2 key

            // VK_OEM_1 (BA) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the ';:' key
        case Qt::Key_Semicolon:
        case Qt::Key_Colon:
            return VK_OEM_1; // case ';': case ':': return 0xBA;
            // VK_OEM_PLUS (BB) Windows 2000/XP: For any country/region, the '+' key
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            return VK_OEM_PLUS; // case '=': case '+': return 0xBB;
            // VK_OEM_COMMA (BC) Windows 2000/XP: For any country/region, the ',' key
        case Qt::Key_Comma:
        case Qt::Key_Less:
            return VK_OEM_COMMA; // case ',': case '<': return 0xBC;
            // VK_OEM_MINUS (BD) Windows 2000/XP: For any country/region, the '-' key
        case Qt::Key_Minus:
        case Qt::Key_Underscore:
            return VK_OEM_MINUS; // case '-': case '_': return 0xBD;
            // VK_OEM_PERIOD (BE) Windows 2000/XP: For any country/region, the '.' key
        case Qt::Key_Period:
        case Qt::Key_Greater:
            return VK_OEM_PERIOD; // case '.': case '>': return 0xBE;
            // VK_OEM_2 (BF) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '/?' key
        case Qt::Key_Slash:
        case Qt::Key_Question:
            return VK_OEM_2; // case '/': case '?': return 0xBF;
            // VK_OEM_3 (C0) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '`~' key
        case Qt::Key_AsciiTilde:
        case Qt::Key_QuoteLeft:
            return VK_OEM_3; // case '`': case '~': return 0xC0;
            // VK_OEM_4 (DB) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '[{' key
        case Qt::Key_BracketLeft:
        case Qt::Key_BraceLeft:
            return VK_OEM_4; // case '[': case '{': return 0xDB;
            // VK_OEM_5 (DC) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '\|' key
        case Qt::Key_Backslash:
        case Qt::Key_Bar:
            return VK_OEM_5; // case '\\': case '|': return 0xDC;
            // VK_OEM_6 (DD) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the ']}' key
        case Qt::Key_BracketRight:
        case Qt::Key_BraceRight:
            return VK_OEM_6; // case ']': case '}': return 0xDD;
            // VK_OEM_7 (DE) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the 'single-quote/double-quote' key
        case Qt::Key_Apostrophe:
        case Qt::Key_QuoteDbl:
            return VK_OEM_7; // case '\'': case '"': return 0xDE;
            // VK_OEM_8 (DF) Used for miscellaneous characters; it can vary by keyboard.
            // VK_OEM_102 (E2) Windows 2000/XP: Either the angle bracket key or the backslash key on the RT 102-key keyboard

        case Qt::Key_AudioRewind:
            return 0xE3; // (E3) Android/GoogleTV: Rewind media key (Windows: VK_ICO_HELP Help key on 1984 Olivetti M24 deluxe keyboard)
        case Qt::Key_AudioForward:
            return 0xE4; // (E4) Android/GoogleTV: Fast forward media key  (Windows: VK_ICO_00 '00' key on 1984 Olivetti M24 deluxe keyboard)

            // VK_PROCESSKEY (E5) Windows 95/98/Me, Windows NT 4.0, Windows 2000/XP: IME PROCESS key
            // VK_PACKET (E7) Windows 2000/XP: Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT,SendInput, WM_KEYDOWN, and WM_KEYUP
            // VK_ATTN (F6) Attn key
            // VK_CRSEL (F7) CrSel key
            // VK_EXSEL (F8) ExSel key
            // VK_EREOF (F9) Erase EOF key
            // VK_PLAY (FA) Play key
            // VK_ZOOM (FB) Zoom key
            // VK_NONAME (FC) Reserved for future use
            // VK_PA1 (FD) PA1 key
            // VK_OEM_CLEAR (FE) Clear key
        default:
            return 0;
        }
    }
}

/*
 * List of the available DOM key values can be found in the
 * chromium/ui/events/keycodes/dom/dom_key_data_inc file.
 * Some of the DOM keys are currently unsupported in Qt:
 *
 *  Modifier Keys:
 *      - ui::DomKey::ACCEL
 *      - ui::DomKey::FN
 *      - ui::DomKey::FN_LOCK
 *      - ui::DomKey::SYMBOL
 *      - ui::DomKey::SYMBOL_LOCK
 *      - ui::DomKey::SHIFT_LEVEL5
 *      - ui::DomKey::ALT_GRAPH_LATCH
 *
 *  Editing Keys:
 *      - ui::DomKey::CR_SEL
 *      - ui::DomKey::ERASE_OF
 *      - ui::DomKey::EX_SEL
 *
 *  UI Keys:
 *      - ui::DomKey::ACCEPT
 *      - ui::DomKey::AGAIN
 *      - ui::DomKey::ATTN
 *      - ui::DomKey::PROPS
 *
 *  Device Keys:
 *      - ui::DomKey::POWER
 *
 *  IME and Composition Keys:
 *      - ui::DomKey::ALL_CANDIDATES
 *      - ui::DomKey::ALPHANUMERIC
 *      - ui::DomKey::FINAL_MODE
 *      - ui::DomKey::GROUP_FIRST
 *      - ui::DomKey::GROUP_LAST
 *      - ui::DomKey::GROUP_NEXT
 *      - ui::DomKey::GROUP_PREVIOUS
 *      - ui::DomKey::NEXT_CANDIDATE
 *      - ui::DomKey::PREVIOUS_CANDIDATE
 *      - ui::DomKey::PROCESS
 *      - ui::DomKey::JUNJA_MODE
 *
 *  General-Purpose Function Keys:
 *      - ui::DomKey::SOFT1
 *      - ui::DomKey::SOFT2
 *      - ui::DomKey::SOFT3
 *      - ui::DomKey::SOFT4
 *      - ui::DomKey::SOFT5
 *      - ui::DomKey::SOFT6
 *      - ui::DomKey::SOFT7
 *      - ui::DomKey::SOFT8
 *
 *  Multimedia Numpad Keys:
 *      - ui::DomKey::KEY11
 *      - ui::DomKey::KEY12
 *
 *  Audio Keys:
 *      - ui::DomKey::AUDIO_BALANCE_LEFT
 *      - ui::DomKey::AUDIO_BALANCE_RIGHT
 *      - ui::DomKey::AUDIO_BASS_BOOST_DOWN
 *      - ui::DomKey::AUDIO_BASS_BOOST_UP
 *      - ui::DomKey::AUDIO_FADER_FRONT
 *      - ui::DomKey::AUDIO_FADER_REAR
 *      - ui::DomKey::AUDIO_SURROUND_MODE_NEXT
 *      - ui::DomKey::MICROPHONE_TOGGLE
 *
 *  Speech Keys:
 *      - ui::DomKey::SPEECH_CORRECTION_LIST
 *      - ui::DomKey::SPEECH_INPUT_TOGGLE
 *
 *  Application Keys:
 *      - ui::DomKey::LAUNCH_CONTACTS
 *
 *  Mobile Phone Keys:
 *      - ui::DomKey::APP_SWITCH
 *      - ui::DomKey::GO_BACK
 *      - ui::DomKey::GO_HOME
 *      - ui::DomKey::HEADSET_HOOK
 *      - ui::DomKey::NOTIFICATION
 *      - ui::DomKey::MANNER_MODE
 *
 *  TV Keys:
 *      - ui::DomKey::TV
 *      - ui::DomKey::TV_3D_MODE
 *      - ui::DomKey::TV_ANTENNA_CABLE
 *      - ui::DomKey::TV_AUDIO_DESCRIPTION
 *      - ui::DomKey::TV_AUDIO_DESCRIPTION_MIX_DOWN
 *      - ui::DomKey::TV_AUDIO_DESCRIPTION_MIX_UP
 *      - ui::DomKey::TV_CONTENTS_MENU
 *      - ui::DomKey::TV_DATA_SERVICE
 *      - ui::DomKey::TV_INPUT
 *      - ui::DomKey::TV_INPUT_COMPONENT1
 *      - ui::DomKey::TV_INPUT_COMPONENT2
 *      - ui::DomKey::TV_INPUT_COMPOSITE1
 *      - ui::DomKey::TV_INPUT_COMPOSITE2
 *      - ui::DomKey::TV_INPUT_HDMI1
 *      - ui::DomKey::TV_INPUT_HDMI2
 *      - ui::DomKey::TV_INPUT_HDMI3
 *      - ui::DomKey::TV_INPUT_HDMI4
 *      - ui::DomKey::TV_INPUT_VGA1
 *      - ui::DomKey::TV_MEDIA_CONTEXT
 *      - ui::DomKey::TV_NETWORK
 *      - ui::DomKey::TV_NUMBER_ENTRY
 *      - ui::DomKey::TV_POWER
 *      - ui::DomKey::TV_RADIO_SERVICE
 *      - ui::DomKey::TV_SATELLITE
 *      - ui::DomKey::TV_SATELLITE_BC
 *      - ui::DomKey::TV_SATELLITE_CS
 *      - ui::DomKey::TV_SATELLITE_TOGGLE
 *      - ui::DomKey::TV_TERRESTRIAL_ANALOG
 *      - ui::DomKey::TV_TERRESTRIAL_DIGITAL
 *      - ui::DomKey::TV_TIMER
 *
 *  Media Controller Keys:
 *      - ui::DomKey::AVR_INPUT
 *      - ui::DomKey::AVR_POWER
 *      - ui::DomKey::COLOR_F4_GREY
 *      - ui::DomKey::COLOR_F5_BROWN
 *      - ui::DomKey::CLOSED_CAPTION_TOGGLE
 *      - ui::DomKey::DVR
 *      - ui::DomKey::FAVORITE_CLEAR0
 *      - ui::DomKey::FAVORITE_CLEAR1
 *      - ui::DomKey::FAVORITE_CLEAR2
 *      - ui::DomKey::FAVORITE_CLEAR3
 *      - ui::DomKey::FAVORITE_RECALL0
 *      - ui::DomKey::FAVORITE_RECALL1
 *      - ui::DomKey::FAVORITE_RECALL2
 *      - ui::DomKey::FAVORITE_RECALL3
 *      - ui::DomKey::FAVORITE_STORE0
 *      - ui::DomKey::FAVORITE_STORE1
 *      - ui::DomKey::FAVORITE_STORE2
 *      - ui::DomKey::FAVORITE_STORE3
 *      - ui::DomKey::GUIDE_NEXT_DAY
 *      - ui::DomKey::GUIDE_PREVIOUS_DAY
 *      - ui::DomKey::INSTANT_REPLAY
 *      - ui::DomKey::LINK
 *      - ui::DomKey::LIST_PROGRAM
 *      - ui::DomKey::LIVE_CONTENT
 *      - ui::DomKey::LOCK
 *      - ui::DomKey::MEDIA_APPS
 *      - ui::DomKey::MEDIA_AUDIO_TRACK
 *      - ui::DomKey::MEDIA_SKIP_BACKWARD
 *      - ui::DomKey::MEDIA_SKIP_FORWARD
 *      - ui::DomKey::MEDIA_SKIP
 *      - ui::DomKey::MEDIA_STEP_BACKWARD
 *      - ui::DomKey::MEDIA_STEP_FORWARD
 *      - ui::DomKey::NAVIGATE_IN
 *      - ui::DomKey::NAVIGATE_NEXT
 *      - ui::DomKey::NAVIGATE_OUT
 *      - ui::DomKey::NAVIGATE_PREVIOUS
 *      - ui::DomKey::NEXT_FAVORITE_CHANNEL
 *      - ui::DomKey::NEXT_USER_PROFILE
 *      - ui::DomKey::ON_DEMAND
 *      - ui::DomKey::PAIRING
 *      - ui::DomKey::PINP_DOWN
 *      - ui::DomKey::PINP_MOVE
 *      - ui::DomKey::PINP_TOGGLE
 *      - ui::DomKey::PINP_UP
 *      - ui::DomKey::PLAY_SPEED_DOWN
 *      - ui::DomKey::PLAY_SPEED_RESET
 *      - ui::DomKey::PLAY_SPEED_UP
 *      - ui::DomKey::RC_LOW_BATTERY
 *      - ui::DomKey::RECORD_SPEED_NEXT
 *      - ui::DomKey::RF_BYPASS
 *      - ui::DomKey::SCAN_CHANNELS_TOGGLE
 *      - ui::DomKey::SCREEN_MODE_NEXT
 *      - ui::DomKey::STB_INPUT
 *      - ui::DomKey::STB_POWER
 *      - ui::DomKey::TELETEXT
 *      - ui::DomKey::VIDEO_MODE_NEXT
 *      - ui::DomKey::WINK
 */
static ui::DomKey domKeyForQtKey(int qtKey)
{
    Q_ASSERT(qtKey >= Qt::Key_Escape);
    switch (qtKey) {
    case Qt::Key_Backspace:
        return ui::DomKey::BACKSPACE;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        return ui::DomKey::TAB;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return ui::DomKey::ENTER;
    case Qt::Key_Escape:
        return ui::DomKey::ESCAPE;
    case Qt::Key_Delete:
        return ui::DomKey::DEL;

    // Special Key Values
    case Qt::Key_unknown:
        return ui::DomKey::UNIDENTIFIED;

    // Modifier Keys
    case Qt::Key_Alt:
        return ui::DomKey::ALT;
    case Qt::Key_AltGr:
        return ui::DomKey::ALT_GRAPH;
    case Qt::Key_CapsLock:
        return ui::DomKey::CAPS_LOCK;
    case Qt::Key_Control:
        return ui::DomKey::CONTROL;
    case Qt::Key_Hyper_L:
    case Qt::Key_Hyper_R:
        return ui::DomKey::HYPER;
    case Qt::Key_Meta:
        return ui::DomKey::META;
    case Qt::Key_NumLock:
        return ui::DomKey::NUM_LOCK;
    case Qt::Key_ScrollLock:
        return ui::DomKey::SCROLL_LOCK;
    case Qt::Key_Shift:
        return ui::DomKey::SHIFT;
    case Qt::Key_Super_L:
    case Qt::Key_Super_R:
        return ui::DomKey::SUPER;

    // Navigation Keys
    case Qt::Key_Down:
        return ui::DomKey::ARROW_DOWN;
    case Qt::Key_Left:
        return ui::DomKey::ARROW_LEFT;
    case Qt::Key_Right:
        return ui::DomKey::ARROW_RIGHT;
    case Qt::Key_Up:
        return ui::DomKey::ARROW_UP;
    case Qt::Key_End:
        return ui::DomKey::END;
    case Qt::Key_Home:
        return ui::DomKey::HOME;
    case Qt::Key_PageUp:
        return ui::DomKey::PAGE_UP;
    case Qt::Key_PageDown:
        return ui::DomKey::PAGE_DOWN;

    // Editing Keys
    case Qt::Key_Clear:
        return ui::DomKey::CLEAR;
    case Qt::Key_Copy:
        return ui::DomKey::COPY;
    case Qt::Key_Cut:
        return ui::DomKey::CUT;
    case Qt::Key_Insert:
        return ui::DomKey::INSERT;
    case Qt::Key_Paste:
        return ui::DomKey::PASTE;
    case Qt::Key_Redo:
        return ui::DomKey::REDO;
    case Qt::Key_Undo:
        return ui::DomKey::UNDO;

    // UI Keys
    case Qt::Key_Cancel:
        return ui::DomKey::CANCEL;
    case Qt::Key_Menu:
        return ui::DomKey::CONTEXT_MENU;
    case Qt::Key_Execute:
        return ui::DomKey::EXECUTE;
    case Qt::Key_Find:
        return ui::DomKey::FIND;
    case Qt::Key_Help:
        return ui::DomKey::HELP;
    case Qt::Key_Pause:
        return ui::DomKey::PAUSE;
    case Qt::Key_Play:
        return ui::DomKey::PLAY;
    case Qt::Key_Select:
        return ui::DomKey::SELECT;
    case Qt::Key_ZoomIn:
        return ui::DomKey::ZOOM_IN;
    case Qt::Key_ZoomOut:
        return ui::DomKey::ZOOM_OUT;

    // Device Keys
    case Qt::Key_MonBrightnessDown:
        return ui::DomKey::BRIGHTNESS_DOWN;
    case Qt::Key_MonBrightnessUp:
        return ui::DomKey::BRIGHTNESS_UP;
    case Qt::Key_Eject:
        return ui::DomKey::EJECT;
    case Qt::Key_LogOff:
        return ui::DomKey::LOG_OFF;
    case Qt::Key_PowerDown:
    case Qt::Key_PowerOff:
        return ui::DomKey::POWER_OFF;
    case Qt::Key_Print:
        return ui::DomKey::PRINT_SCREEN;
    case Qt::Key_Hibernate:
        return ui::DomKey::HIBERNATE;
    case Qt::Key_Standby:
        return ui::DomKey::STANDBY;
    case Qt::Key_WakeUp:
        return ui::DomKey::WAKE_UP;

    // IME and Composition Keys
    case Qt::Key_Codeinput:
        return ui::DomKey::CODE_INPUT;
    case Qt::Key_Multi_key:
        return ui::DomKey::COMPOSE;
    case Qt::Key_Henkan:
        return ui::DomKey::CONVERT;
    case Qt::Key_Mode_switch:
        return ui::DomKey::MODE_CHANGE;
    case Qt::Key_Muhenkan:
        return ui::DomKey::NON_CONVERT;
    case Qt::Key_SingleCandidate:
        return ui::DomKey::SINGLE_CANDIDATE;
    case Qt::Key_Hangul:
        return ui::DomKey::HANGUL_MODE;
    case Qt::Key_Hangul_Hanja:
        return ui::DomKey::HANJA_MODE;
    case Qt::Key_Eisu_Shift:
    case Qt::Key_Eisu_toggle:
        return ui::DomKey::EISU;
    case Qt::Key_Hankaku:
        return ui::DomKey::HANKAKU;
    case Qt::Key_Hiragana:
        return ui::DomKey::HIRAGANA;
    case Qt::Key_Hiragana_Katakana:
        return ui::DomKey::HIRAGANA_KATAKANA;
    case Qt::Key_Kana_Lock:
    case Qt::Key_Kana_Shift:
        return ui::DomKey::KANA_MODE;
    case Qt::Key_Kanji:
        return ui::DomKey::KANJI_MODE;
    case Qt::Key_Katakana:
        return ui::DomKey::KATAKANA;
    case Qt::Key_Romaji:
        return ui::DomKey::ROMAJI;
    case Qt::Key_Zenkaku:
        return ui::DomKey::ZENKAKU;
    case Qt::Key_Zenkaku_Hankaku:
        return ui::DomKey::ZENKAKU_HANKAKU;
    // Dead keys (ui/events/keycodes/keyboard_code_conversion_xkb.cc)
    case Qt::Key_Dead_Grave:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0300);
    case Qt::Key_Dead_Acute:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0301);
    case Qt::Key_Dead_Circumflex:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0302);
    case Qt::Key_Dead_Tilde:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0303);
    case Qt::Key_Dead_Macron:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0304);
    case Qt::Key_Dead_Breve:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0306);
    case Qt::Key_Dead_Abovedot:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0307);
    case Qt::Key_Dead_Diaeresis:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0308);
    case Qt::Key_Dead_Abovering:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x030A);
    case Qt::Key_Dead_Doubleacute:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x030B);
    case Qt::Key_Dead_Caron:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x030C);
    case Qt::Key_Dead_Cedilla:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0327);
    case Qt::Key_Dead_Ogonek:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0328);
    case Qt::Key_Dead_Iota:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0345);
    case Qt::Key_Dead_Voiced_Sound:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x3099);
    case Qt::Key_Dead_Semivoiced_Sound:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x309A);
    case Qt::Key_Dead_Belowdot:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0323);
    case Qt::Key_Dead_Hook:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0309);
    case Qt::Key_Dead_Horn:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x031B);
    case Qt::Key_Dead_Stroke:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0338);
    case Qt::Key_Dead_Abovecomma:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0313);
    case Qt::Key_Dead_Abovereversedcomma:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0314);
    case Qt::Key_Dead_Doublegrave:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x030F);
    case Qt::Key_Dead_Belowring:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0325);
    case Qt::Key_Dead_Belowmacron:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0331);
    case Qt::Key_Dead_Belowcircumflex:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x032D);
    case Qt::Key_Dead_Belowtilde:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0330);
    case Qt::Key_Dead_Belowbreve:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x032E);
    case Qt::Key_Dead_Belowdiaeresis:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0324);
    case Qt::Key_Dead_Invertedbreve:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0311);
    case Qt::Key_Dead_Belowcomma:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x0326);
    case Qt::Key_Dead_Currency:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x00A4);
    case Qt::Key_Dead_Greek:
        return ui::DomKey::DeadKeyFromCombiningCharacter(0x037E);
    // General-Purpose Function Keys
    case Qt::Key_F1:
        return ui::DomKey::F1;
    case Qt::Key_F2:
        return ui::DomKey::F2;
    case Qt::Key_F3:
        return ui::DomKey::F3;
    case Qt::Key_F4:
        return ui::DomKey::F4;
    case Qt::Key_F5:
        return ui::DomKey::F5;
    case Qt::Key_F6:
        return ui::DomKey::F6;
    case Qt::Key_F7:
        return ui::DomKey::F7;
    case Qt::Key_F8:
        return ui::DomKey::F8;
    case Qt::Key_F9:
        return ui::DomKey::F9;
    case Qt::Key_F10:
        return ui::DomKey::F10;
    case Qt::Key_F11:
        return ui::DomKey::F11;
    case Qt::Key_F12:
        return ui::DomKey::F12;
    case Qt::Key_F13:
        return ui::DomKey::F13;
    case Qt::Key_F14:
        return ui::DomKey::F14;
    case Qt::Key_F15:
        return ui::DomKey::F15;
    case Qt::Key_F16:
        return ui::DomKey::F16;
    case Qt::Key_F17:
        return ui::DomKey::F17;
    case Qt::Key_F18:
        return ui::DomKey::F18;
    case Qt::Key_F19:
        return ui::DomKey::F19;
    case Qt::Key_F20:
        return ui::DomKey::F20;
    case Qt::Key_F21:
        return ui::DomKey::F21;
    case Qt::Key_F22:
        return ui::DomKey::F22;
    case Qt::Key_F23:
        return ui::DomKey::F23;
    case Qt::Key_F24:
        return ui::DomKey::F24;

    // Multimedia Keys
    case Qt::Key_ChannelDown:
        return ui::DomKey::CHANNEL_DOWN;
    case Qt::Key_ChannelUp:
        return ui::DomKey::CHANNEL_UP;
    case Qt::Key_Close:
        return ui::DomKey::CLOSE;
    case Qt::Key_MailForward:
        return ui::DomKey::MAIL_FORWARD;
    case Qt::Key_Reply:
        return ui::DomKey::MAIL_REPLY;
    case Qt::Key_Send:
        return ui::DomKey::MAIL_SEND;
    case Qt::Key_AudioForward:
        return ui::DomKey::MEDIA_FAST_FORWARD;
    case Qt::Key_MediaPause:
        return ui::DomKey::MEDIA_PAUSE;
    case Qt::Key_MediaPlay:
        return ui::DomKey::MEDIA_PLAY;
    case Qt::Key_MediaTogglePlayPause:
        return ui::DomKey::MEDIA_PLAY_PAUSE;
    case Qt::Key_MediaRecord:
        return ui::DomKey::MEDIA_RECORD;
    case Qt::Key_AudioRewind:
        return ui::DomKey::MEDIA_REWIND;
    case Qt::Key_MediaStop:
        return ui::DomKey::MEDIA_STOP;
    case Qt::Key_MediaNext:
        return ui::DomKey::MEDIA_TRACK_NEXT;
    case Qt::Key_MediaPrevious:
        return ui::DomKey::MEDIA_TRACK_PREVIOUS;
    case Qt::Key_New:
        return ui::DomKey::NEW;
    case Qt::Key_Open:
        return ui::DomKey::OPEN;
    case Qt::Key_Printer:
        return ui::DomKey::PRINT;
    case Qt::Key_Save:
        return ui::DomKey::SAVE;
    case Qt::Key_Spell:
        return ui::DomKey::SPELL_CHECK;

    // Audio Keys
    case Qt::Key_BassDown:
        return ui::DomKey::AUDIO_BASS_BOOST_DOWN;
    case Qt::Key_BassBoost:
        return ui::DomKey::AUDIO_BASS_BOOST_TOGGLE;
    case Qt::Key_BassUp:
        return ui::DomKey::AUDIO_BASS_BOOST_UP;
    case Qt::Key_TrebleDown:
        return ui::DomKey::AUDIO_TREBLE_DOWN;
    case Qt::Key_TrebleUp:
        return ui::DomKey::AUDIO_TREBLE_UP;
    case Qt::Key_VolumeDown:
        return ui::DomKey::AUDIO_VOLUME_DOWN;
    case Qt::Key_VolumeUp:
        return ui::DomKey::AUDIO_VOLUME_UP;
    case Qt::Key_VolumeMute:
        return ui::DomKey::AUDIO_VOLUME_MUTE;
    case Qt::Key_MicVolumeDown:
        return ui::DomKey::MICROPHONE_VOLUME_DOWN;
    case Qt::Key_MicVolumeUp:
        return ui::DomKey::MICROPHONE_VOLUME_UP;
    case Qt::Key_MicMute:
        return ui::DomKey::MICROPHONE_VOLUME_MUTE;

    // Application Keys
    case Qt::Key_Calculator:
        return ui::DomKey::LAUNCH_CALCULATOR;
    case Qt::Key_Calendar:
        return ui::DomKey::LAUNCH_CALENDAR;
    case Qt::Key_LaunchMail:
        return ui::DomKey::LAUNCH_MAIL;
    case Qt::Key_LaunchMedia:
        return ui::DomKey::LAUNCH_MEDIA_PLAYER;
    case Qt::Key_Music:
        return ui::DomKey::LAUNCH_MUSIC_PLAYER;
    case Qt::Key_Launch0:
        return ui::DomKey::LAUNCH_MY_COMPUTER;
    case Qt::Key_Phone:
        return ui::DomKey::LAUNCH_PHONE;
    case Qt::Key_ScreenSaver:
        return ui::DomKey::LAUNCH_SCREEN_SAVER;
    case Qt::Key_Excel:
        return ui::DomKey::LAUNCH_SPREADSHEET;
    case Qt::Key_WWW:
        return ui::DomKey::LAUNCH_WEB_BROWSER;
    case Qt::Key_WebCam:
        return ui::DomKey::LAUNCH_WEB_CAM;
    case Qt::Key_Word:
        return ui::DomKey::LAUNCH_WORD_PROCESSOR;

    // Browser Keys
    case Qt::Key_Back:
        return ui::DomKey::BROWSER_BACK;
    case Qt::Key_Favorites:
        return ui::DomKey::BROWSER_FAVORITES;
    case Qt::Key_Forward:
        return ui::DomKey::BROWSER_FORWARD;
    case Qt::Key_HomePage:
        return ui::DomKey::BROWSER_HOME;
    case Qt::Key_Refresh:
        return ui::DomKey::BROWSER_REFRESH;
    case Qt::Key_Search:
        return ui::DomKey::BROWSER_SEARCH;
    case Qt::Key_Stop:
        return ui::DomKey::BROWSER_STOP;

    // Mobile Phone Keys
    case Qt::Key_Call:
        return ui::DomKey::CALL;
    case Qt::Key_Camera:
        return ui::DomKey::CAMERA;
    case Qt::Key_CameraFocus:
        return ui::DomKey::CAMERA_FOCUS;
    case Qt::Key_Hangup:
        return ui::DomKey::END_CALL;
    case Qt::Key_LastNumberRedial:
        return ui::DomKey::LAST_NUMBER_REDIAL;
    case Qt::Key_VoiceDial:
        return ui::DomKey::VOICE_DIAL;

    // Media Controller Keys
    case Qt::Key_Red:
        return ui::DomKey::COLOR_F0_RED;
    case Qt::Key_Green:
        return ui::DomKey::COLOR_F1_GREEN;
    case Qt::Key_Yellow:
        return ui::DomKey::COLOR_F2_YELLOW;
    case Qt::Key_Blue:
        return ui::DomKey::COLOR_F3_BLUE;
    case Qt::Key_BrightnessAdjust:
        return ui::DomKey::DIMMER;
    case Qt::Key_Display:
        return ui::DomKey::DISPLAY_SWAP;
    case Qt::Key_Exit:
        return ui::DomKey::EXIT;
    case Qt::Key_Guide:
        return ui::DomKey::GUIDE;
    case Qt::Key_Info:
        return ui::DomKey::INFO;
    case Qt::Key_MediaLast:
        return ui::DomKey::MEDIA_LAST;
    case Qt::Key_TopMenu:
        return ui::DomKey::MEDIA_TOP_MENU;
    case Qt::Key_AudioRandomPlay:
        return ui::DomKey::RANDOM_TOGGLE;
    case Qt::Key_Settings:
        return ui::DomKey::SETTINGS;
    case Qt::Key_SplitScreen:
        return ui::DomKey::SPLIT_SCREEN_TOGGLE;
    case Qt::Key_Subtitle:
        return ui::DomKey::SUBTITLE;
    case Qt::Key_Zoom:
        return ui::DomKey::ZOOM_TOGGLE;

    default:
        return ui::DomKey::NONE;
    }
}

template<class T>
static WebMouseEvent::Button mouseButtonForEvent(T *event)
{
    if (event->button() == Qt::LeftButton)
        return WebMouseEvent::Button::kLeft;
    else if (event->button() == Qt::RightButton)
        return WebMouseEvent::Button::kRight;
    else if (event->button() == Qt::MiddleButton)
        return WebMouseEvent::Button::kMiddle;
    else if (event->button() == Qt::BackButton)
        return WebMouseEvent::Button::kBack;
    else if (event->button() == Qt::ForwardButton)
        return WebMouseEvent::Button::kForward;

    if (event->type() != QEvent::MouseMove && event->type() != QEvent::TabletMove)
        return WebMouseEvent::Button::kNoButton;

    // This is technically wrong, mouse move should always have ButtonNone,
    // but it is consistent with aura and selection code depends on it:
    if (event->buttons() & Qt::LeftButton)
        return WebMouseEvent::Button::kLeft;
    else if (event->buttons() & Qt::RightButton)
        return WebMouseEvent::Button::kRight;
    else if (event->buttons() & Qt::MiddleButton)
        return WebMouseEvent::Button::kMiddle;
    else if (event->buttons() & Qt::BackButton)
        return WebMouseEvent::Button::kBack;
    else if (event->buttons() & Qt::ForwardButton)
        return WebMouseEvent::Button::kForward;

    return WebMouseEvent::Button::kNoButton;
}

template <typename T>
static unsigned mouseButtonsModifiersForEvent(const T* event)
{
    unsigned ret = 0;
    if (event->buttons() & Qt::LeftButton)
        ret |= WebInputEvent::kLeftButtonDown;
    if (event->buttons() & Qt::RightButton)
        ret |= WebInputEvent::kRightButtonDown;
    if (event->buttons() & Qt::MiddleButton)
        ret |= WebInputEvent::kMiddleButtonDown;
    if (event->buttons() & Qt::BackButton)
        ret |= WebInputEvent::kBackButtonDown;
    if (event->buttons() & Qt::ForwardButton)
        ret |= WebInputEvent::kForwardButtonDown;
    return ret;
}

static WebInputEvent::Modifiers lockKeyModifiers(const quint32 nativeModifiers)
{
    unsigned result = 0;
    if (keyboardDriver() == KeyboardDriver::Xkb) {
        if (nativeModifiers & 0x42) /* Caps_Lock */
            result |= WebInputEvent::kCapsLockOn;
        if (nativeModifiers & 0x4d) /* Num_Lock */
            result |= WebInputEvent::kNumLockOn;
    } else if (keyboardDriver() == KeyboardDriver::Windows) {
        if (nativeModifiers & 0x100) /* CapsLock */
            result |= WebInputEvent::kCapsLockOn;
        if (nativeModifiers & 0x200) /* NumLock */
            result |= WebInputEvent::kNumLockOn;
        if (nativeModifiers & 0x400) /* ScrollLock */
            result |= WebInputEvent::kScrollLockOn;
    } else if (keyboardDriver() == KeyboardDriver::Cocoa) {
        if (nativeModifiers & 0x10000) /* NSEventModifierFlagCapsLock */
            result |= WebInputEvent::kCapsLockOn;
    }

    return static_cast<WebInputEvent::Modifiers>(result);
}

// If only a modifier key is pressed, Qt only reports the key code.
// But Chromium also expects the modifier being set.
static inline WebInputEvent::Modifiers modifierForKeyCode(int key)
{
    switch (key) {
        case Qt::Key_Shift:
            return WebInputEvent::kShiftKey;
        case Qt::Key_Alt:
            return WebInputEvent::kAltKey;
        case Qt::Key_Control:
            return WebInputEvent::kControlKey;
        case Qt::Key_Meta:
            return WebInputEvent::kMetaKey;
        default:
            return static_cast<WebInputEvent::Modifiers>(0);
    }
}

static inline WebInputEvent::Modifiers modifiersForEvent(const QInputEvent* event)
{
    unsigned result = 0;
    Qt::KeyboardModifiers modifiers = qtModifiersForEvent(event);
    if (modifiers & Qt::ControlModifier)
        result |= WebInputEvent::kControlKey;
    if (modifiers & Qt::MetaModifier)
        result |= WebInputEvent::kMetaKey;
    if (modifiers & Qt::ShiftModifier)
        result |= WebInputEvent::kShiftKey;
    if (modifiers & Qt::AltModifier)
        result |= WebInputEvent::kAltKey;
    if (modifiers & Qt::KeypadModifier)
        result |= WebInputEvent::kIsKeyPad;

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
        result |= mouseButtonsModifiersForEvent(static_cast<const QMouseEvent*>(event));
        break;
    case QEvent::Wheel:
        result |= mouseButtonsModifiersForEvent(static_cast<const QWheelEvent*>(event));
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        const QKeyEvent *keyEvent = static_cast<const QKeyEvent*>(event);
        if (keyEvent->isAutoRepeat())
            result |= WebInputEvent::kIsAutoRepeat;
        result |= modifierForKeyCode(qtKeyForKeyEvent(keyEvent));
        result |= lockKeyModifiers(keyEvent->nativeModifiers());
        break;
    }
    default:
        break;
    }

    return static_cast<WebInputEvent::Modifiers>(result);
}

static inline Qt::KeyboardModifiers keyboardModifiersForModifier(unsigned int modifier)
{
    Qt::KeyboardModifiers modifiers = {};
    if (modifier & WebInputEvent::kControlKey)
        modifiers |= Qt::ControlModifier;
    if (modifier & WebInputEvent::kMetaKey)
        modifiers |= Qt::MetaModifier;
    if (modifier & WebInputEvent::kShiftKey)
        modifiers |= Qt::ShiftModifier;
    if (modifier & WebInputEvent::kAltKey)
        modifiers |= Qt::AltModifier;
    if (modifier & WebInputEvent::kIsKeyPad)
        modifiers |= Qt::KeypadModifier;

    if (keyboardDriver() == KeyboardDriver::Cocoa && !qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)) {
        bool controlModifier = modifiers.testFlag(Qt::ControlModifier);
        bool metaModifier = modifiers.testFlag(Qt::MetaModifier);
        modifiers.setFlag(Qt::ControlModifier, metaModifier);
        modifiers.setFlag(Qt::MetaModifier, controlModifier);
    }

    return modifiers;
}

static inline Qt::MouseButtons mouseButtonsForModifier(unsigned int modifier)
{
    Qt::MouseButtons buttons = {};
    if (modifier & WebInputEvent::kLeftButtonDown)
        buttons |= Qt::LeftButton;
    if (modifier & WebInputEvent::kRightButtonDown)
        buttons |= Qt::RightButton;
    if (modifier & WebInputEvent::kMiddleButtonDown)
        buttons |= Qt::MiddleButton;
    if (modifier & WebInputEvent::kBackButtonDown)
        buttons |= Qt::BackButton;
    if (modifier & WebInputEvent::kForwardButtonDown)
        buttons |= Qt::ForwardButton;
    return buttons;
}

static WebInputEvent::Type webEventTypeForEvent(const QEvent* event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::TabletPress:
        return WebInputEvent::Type::kMouseDown;
    case QEvent::MouseButtonRelease:
    case QEvent::TabletRelease:
        return WebInputEvent::Type::kMouseUp;
    case QEvent::Enter:
        return WebInputEvent::Type::kMouseEnter;
    case QEvent::Leave:
        return WebInputEvent::Type::kMouseLeave;
    case QEvent::MouseMove:
    case QEvent::TabletMove:
        return WebInputEvent::Type::kMouseMove;
    case QEvent::Wheel:
        return WebInputEvent::Type::kMouseWheel;
    case QEvent::KeyPress:
        return WebInputEvent::Type::kRawKeyDown;
    case QEvent::KeyRelease:
        return WebInputEvent::Type::kKeyUp;
    case QEvent::HoverMove:
        return WebInputEvent::Type::kMouseMove;
    case QEvent::TouchBegin:
        return WebInputEvent::Type::kTouchStart;
    case QEvent::TouchUpdate:
        return WebInputEvent::Type::kTouchMove;
    case QEvent::TouchEnd:
        return WebInputEvent::Type::kTouchEnd;
    case QEvent::TouchCancel:
        return WebInputEvent::Type::kTouchCancel;
    default:
        Q_ASSERT(false);
        return WebInputEvent::Type::kMouseMove;
    }
}

#if QT_CONFIG(tabletevent)
static WebPointerProperties::PointerType pointerTypeForTabletEvent(const QTabletEvent *ev)
{
    switch (ev->pointerType()) {
    case QPointingDevice::PointerType::Unknown:
        return WebPointerProperties::PointerType::kUnknown;
    case QPointingDevice::PointerType::Pen:
        return WebPointerProperties::PointerType::kPen;
    case QPointingDevice::PointerType::Eraser:
        return WebPointerProperties::PointerType::kEraser;
    case QPointingDevice::PointerType::Finger:
        return WebPointerProperties::PointerType::kTouch;
    default:
        return WebPointerProperties::PointerType::kMouse;
    }
}
#endif

WebMouseEvent WebEventFactory::toWebMouseEvent(QMouseEvent *ev)
{
    WebMouseEvent webKitEvent(webEventTypeForEvent(ev),
                              gfx::PointF(ev->position().x(), ev->position().y()),
                              gfx::PointF(ev->globalPosition().x(), ev->globalPosition().y()),
                              mouseButtonForEvent<QMouseEvent>(ev),
                              0,
                              modifiersForEvent(ev),
                              base::TimeTicks::Now());

    webKitEvent.pointer_type = WebPointerProperties::PointerType::kMouse;

    return webKitEvent;
}

WebMouseEvent WebEventFactory::toWebMouseEvent(QHoverEvent *ev)
{
    WebMouseEvent webKitEvent;
    webKitEvent.SetTimeStamp(base::TimeTicks::Now());
    webKitEvent.SetModifiers(modifiersForEvent(ev));
    webKitEvent.SetType(webEventTypeForEvent(ev));

    webKitEvent.SetPositionInWidget(ev->position().x(), ev->position().y());
    webKitEvent.movement_x = ev->position().x() - ev->oldPos().x();
    webKitEvent.movement_y = ev->position().y() - ev->oldPos().y();
    webKitEvent.is_raw_movement_event = true;
    webKitEvent.pointer_type = WebPointerProperties::PointerType::kMouse;

    return webKitEvent;
}

#if QT_CONFIG(tabletevent)
WebMouseEvent WebEventFactory::toWebMouseEvent(QTabletEvent *ev)
{
    WebMouseEvent webKitEvent(webEventTypeForEvent(ev),
                              gfx::PointF(ev->position().x(), ev->position().y()),
                              gfx::PointF(ev->globalPosition().x(), ev->globalPosition().y()),
                              mouseButtonForEvent<QTabletEvent>(ev),
                              0,
                              modifiersForEvent(ev),
                              base::TimeTicks::Now());

    webKitEvent.force = ev->pressure();
    webKitEvent.tilt_x = ev->xTilt();
    webKitEvent.tilt_y = ev->yTilt();
    webKitEvent.tangential_pressure = ev->tangentialPressure();
    webKitEvent.twist = ev->rotation();
    webKitEvent.pointer_type = pointerTypeForTabletEvent(ev);
    return webKitEvent;
}
#endif

WebMouseEvent WebEventFactory::toWebMouseEvent(QEvent *ev)
{
    Q_ASSERT(ev->type() == QEvent::Leave || ev->type() == QEvent::HoverLeave);

    WebMouseEvent webKitEvent;
    webKitEvent.SetTimeStamp(base::TimeTicks::Now());
    webKitEvent.SetType(WebInputEvent::Type::kMouseLeave);
    return webKitEvent;
}

#if QT_CONFIG(gestures)
WebGestureEvent WebEventFactory::toWebGestureEvent(QNativeGestureEvent *ev)
{
    WebGestureEvent webKitEvent;
    webKitEvent.SetTimeStamp(base::TimeTicks::Now());
    webKitEvent.SetModifiers(modifiersForEvent(ev));

    webKitEvent.SetPositionInWidget(gfx::PointF(ev->position().x(),
                                                ev->position().y()));

    webKitEvent.SetPositionInScreen(gfx::PointF(ev->globalPosition().x(),
                                                ev->globalPosition().y()));

    webKitEvent.SetSourceDevice(blink::WebGestureDevice::kTouchpad);

    Qt::NativeGestureType gestureType = ev->gestureType();
    switch (gestureType) {
    case Qt::ZoomNativeGesture:
        webKitEvent.SetType(WebInputEvent::Type::kGesturePinchUpdate);
        webKitEvent.data.pinch_update.scale = static_cast<float>(ev->value() + 1.0);
        break;
    case Qt::SmartZoomNativeGesture:
        webKitEvent.SetType(WebInputEvent::Type::kGestureDoubleTap);
        webKitEvent.data.tap.tap_count = 1;
        break;
    case Qt::BeginNativeGesture:
        webKitEvent.SetType(WebInputEvent::Type::kGesturePinchBegin);
        webKitEvent.SetNeedsWheelEvent(true);
        break;
    case Qt::EndNativeGesture:
        webKitEvent.SetType(WebInputEvent::Type::kGesturePinchEnd);
        webKitEvent.SetNeedsWheelEvent(true);
        break;
    case Qt::RotateNativeGesture:
    case Qt::PanNativeGesture:
    case Qt::SwipeNativeGesture:
        // Not implemented by Chromium for now.
        webKitEvent.SetType(blink::WebInputEvent::Type::kUndefined);
        break;
    }

    return webKitEvent;
}
#endif

static void setBlinkWheelEventDelta(blink::WebMouseWheelEvent &webEvent)
{
    // We can't use the device specific QWheelEvent::pixelDelta(), so we calculate
    // a pixel delta based on ticks and scroll per line.
    static const float cDefaultQtScrollStep = 20.f;

    static const int wheelScrollLines = QGuiApplication::styleHints()->wheelScrollLines();
    webEvent.delta_x = webEvent.wheel_ticks_x * wheelScrollLines * cDefaultQtScrollStep;
    webEvent.delta_y = webEvent.wheel_ticks_y * wheelScrollLines * cDefaultQtScrollStep;
}

static QPoint getWheelEventDelta(const blink::WebGestureEvent &webEvent)
{
    static const float cDefaultQtScrollStep = 20.f;
    static const int wheelScrollLines = QGuiApplication::styleHints()->wheelScrollLines();
    return QPoint(webEvent.data.scroll_update.delta_x * QWheelEvent::DefaultDeltasPerStep / (wheelScrollLines * cDefaultQtScrollStep),
                  webEvent.data.scroll_update.delta_y * QWheelEvent::DefaultDeltasPerStep / (wheelScrollLines * cDefaultQtScrollStep));
}

blink::WebMouseWheelEvent::Phase toBlinkPhase(QWheelEvent *ev)
{
    switch (ev->phase()) {
    case Qt::NoScrollPhase:
    case Qt::ScrollMomentum:
        return blink::WebMouseWheelEvent::kPhaseNone;
    case Qt::ScrollBegin:
        return blink::WebMouseWheelEvent::kPhaseBegan;
    case Qt::ScrollUpdate:
        return blink::WebMouseWheelEvent::kPhaseChanged;
    case Qt::ScrollEnd:
        return blink::WebMouseWheelEvent::kPhaseEnded;
    }
    Q_UNREACHABLE();
    return blink::WebMouseWheelEvent::kPhaseNone;
}

blink::WebMouseWheelEvent WebEventFactory::toWebWheelEvent(QWheelEvent *ev)
{
    WebMouseWheelEvent webEvent;
    webEvent.SetType(webEventTypeForEvent(ev));
    webEvent.SetModifiers(modifiersForEvent(ev));
    webEvent.SetTimeStamp(base::TimeTicks::Now());
    webEvent.SetPositionInWidget(static_cast<float>(ev->position().x()),
                                 static_cast<float>(ev->position().y()));
    webEvent.SetPositionInScreen(static_cast<float>(ev->globalPosition().x()),
                                 static_cast<float>(ev->globalPosition().y()));

    webEvent.wheel_ticks_x = static_cast<float>(ev->angleDelta().x()) / QWheelEvent::DefaultDeltasPerStep;
    webEvent.wheel_ticks_y = static_cast<float>(ev->angleDelta().y()) / QWheelEvent::DefaultDeltasPerStep;
    webEvent.phase = toBlinkPhase(ev);
#if defined(Q_OS_DARWIN)
    // PrecisePixel is a macOS term meaning it is a system scroll gesture, see qnsview_mouse.mm
    if (ev->source() == Qt::MouseEventSynthesizedBySystem)
        webEvent.delta_units = ui::ScrollGranularity::kScrollByPrecisePixel;
#endif

    setBlinkWheelEventDelta(webEvent);

    return webEvent;
}

bool WebEventFactory::coalesceWebWheelEvent(blink::WebMouseWheelEvent &webEvent, QWheelEvent *ev)
{
    if (webEventTypeForEvent(ev) != webEvent.GetType())
        return false;
    if (modifiersForEvent(ev) != webEvent.GetModifiers())
        return false;
    if (toBlinkPhase(ev) != webEvent.phase)
        return false;
#if defined(Q_OS_DARWIN)
    if ((webEvent.delta_units == ui::ScrollGranularity::kScrollByPrecisePixel)
            != (ev->source() == Qt::MouseEventSynthesizedBySystem))
        return false;
#endif

    webEvent.SetTimeStamp(base::TimeTicks::Now());
    webEvent.SetPositionInWidget(static_cast<float>(ev->position().x()),
                                 static_cast<float>(ev->position().y()));
    webEvent.SetPositionInScreen(static_cast<float>(ev->globalPosition().x()),
                                 static_cast<float>(ev->globalPosition().y()));

    webEvent.wheel_ticks_x += static_cast<float>(ev->angleDelta().x()) / QWheelEvent::DefaultDeltasPerStep;
    webEvent.wheel_ticks_y += static_cast<float>(ev->angleDelta().y()) / QWheelEvent::DefaultDeltasPerStep;
    setBlinkWheelEventDelta(webEvent);

    return true;
}

static QPointF toQt(gfx::PointF p)
{
    return QPointF(p.x(), p.y());
}

void WebEventFactory::sendUnhandledWheelEvent(const blink::WebGestureEvent &event,
                                              RenderWidgetHostViewQtDelegate *delegate)
{
    Q_ASSERT(event.GetType() == blink::WebInputEvent::Type::kGestureScrollUpdate);

    QWheelEvent ev(toQt(event.PositionInWidget()),
                   toQt(event.PositionInScreen()),
                   QPoint(event.data.scroll_update.delta_x, event.data.scroll_update.delta_y),
                   getWheelEventDelta(event),
                   mouseButtonsForModifier(event.GetModifiers()),
                   keyboardModifiersForModifier(event.GetModifiers()),
                   Qt::NoScrollPhase, false);
    delegate->unhandledWheelEvent(&ev);
}

content::NativeWebKeyboardEvent WebEventFactory::toWebKeyboardEvent(QKeyEvent *ev)
{
    content::NativeWebKeyboardEvent webKitEvent(reinterpret_cast<gfx::NativeEvent>(ev));
    webKitEvent.SetTimeStamp(base::TimeTicks::Now());
    webKitEvent.SetModifiers(modifiersForEvent(ev));
    webKitEvent.SetType(webEventTypeForEvent(ev));

    int qtKey = qtKeyForKeyEvent(ev);
    Qt::KeyboardModifiers qtModifiers = qtModifiersForEvent(ev);
    QString qtText = qtTextForKeyEvent(ev, qtKey, qtModifiers);

    webKitEvent.native_key_code = nativeKeyCodeForKeyEvent(ev);
    webKitEvent.windows_key_code = windowsKeyCodeForQtKey(qtKey, qtModifiers & Qt::KeypadModifier);

    if (qtKey >= Qt::Key_Escape)
        webKitEvent.dom_key = domKeyForQtKey(qtKey);
    else if (!qtText.isEmpty())
        webKitEvent.dom_key = ui::DomKey::FromCharacter(qtText.toUcs4().first());
    else {
        QChar ch(qtKey);
        if (!(qtModifiers & Qt::ShiftModifier)) // No way to check for caps lock
            ch = ch.toLower();
        webKitEvent.dom_key = ui::DomKey::FromCharacter(ch.unicode());
    }

    // The dom_code field should contain the USB keycode of the *physical* key
    // that was pressed. Physical meaning independent of layout and modifiers.
    // Since this information is not available from QKeyEvent in portable form,
    // we try to compute it from the native key code.
    if (webKitEvent.native_key_code)
        webKitEvent.dom_code = static_cast<int>(
            ui::KeycodeConverter::NativeKeycodeToDomCode(webKitEvent.native_key_code));

    // The dom_code and windows_key_code can be converted to each other. The
    // result will be incorrect on non-US layouts.
    if (!webKitEvent.dom_code && webKitEvent.windows_key_code)
        webKitEvent.dom_code = static_cast<int>(
                ui::UsLayoutKeyboardCodeToDomCode(static_cast<ui::KeyboardCode>(webKitEvent.windows_key_code)));
    else if (webKitEvent.dom_code && !webKitEvent.windows_key_code)
        webKitEvent.windows_key_code =
                ui::DomCodeToUsLayoutKeyboardCode(static_cast<ui::DomCode>(webKitEvent.dom_code));

    const ushort* text = qtText.utf16();
    size_t textSize = std::min(sizeof(webKitEvent.text), size_t(qtText.length() * 2));
    memcpy(&webKitEvent.text, text, textSize);
    memcpy(&webKitEvent.unmodified_text, text, textSize);

    if (webKitEvent.windows_key_code == VK_RETURN) {
        // This is the same behavior as GTK:
        // We need to treat the enter key as a key press of character \r. This
        // is apparently just how webkit handles it and what it expects.
        webKitEvent.unmodified_text[0] = '\r';
        webKitEvent.text[0] = webKitEvent.unmodified_text[0];
    }

    return webKitEvent;
}

bool WebEventFactory::getEditCommand(QKeyEvent *event, std::string *editCommand)
{
    // Assign Qt standard key bindings to blink editor commands. Editor command names
    // come from chromium/third_party/WebKit/Source/editing/commands/EditorCommandNames.h
    static struct {
        QKeySequence::StandardKey standardKey;
        std::string name;
    } editCommands[] = {
        { QKeySequence::Delete, "Delete" },
        { QKeySequence::Cut, "Cut" },
        { QKeySequence::Copy, "Copy" },
        { QKeySequence::Paste, "Paste" },
        { QKeySequence::Undo, "Undo" },
        { QKeySequence::Redo, "Redo" },
        { QKeySequence::SelectAll, "SelectAll" },
        { QKeySequence::Bold, "Bold" },
        { QKeySequence::Italic, "Italic" },
        { QKeySequence::Underline, "Underline" },

        { QKeySequence::MoveToNextChar, "MoveRight" },
        { QKeySequence::MoveToPreviousChar, "MoveLeft" },
        { QKeySequence::MoveToNextWord, "MoveWordForward" },
        { QKeySequence::MoveToPreviousWord, "MoveWordBackward" },
        { QKeySequence::MoveToNextLine, "MoveDown" },
        { QKeySequence::MoveToPreviousLine, "MoveUp" },
        { QKeySequence::MoveToNextPage, "MovePageDown" },
        { QKeySequence::MoveToPreviousPage, "MovePageUp" },
        { QKeySequence::MoveToStartOfLine, "MoveToBeginningOfLine" },
        { QKeySequence::MoveToEndOfLine, "MoveToEndOfLine" },
        { QKeySequence::MoveToStartOfBlock, "MoveToBeginningOfParagraph" },
        { QKeySequence::MoveToEndOfBlock, "MoveToEndOfParagraph" },
        { QKeySequence::MoveToStartOfDocument, "MoveToBeginningOfDocument" },
        { QKeySequence::MoveToEndOfDocument, "MoveToEndOfDocument" },

        { QKeySequence::SelectNextChar, "MoveRightAndModifySelection" },
        { QKeySequence::SelectPreviousChar, "MoveLeftAndModifySelection" },
        { QKeySequence::SelectNextWord, "MoveWordForwardAndModifySelection" },
        { QKeySequence::SelectPreviousWord, "MoveWordBackwardAndModifySelection" },
        { QKeySequence::SelectNextLine, "MoveDownAndModifySelection" },
        { QKeySequence::SelectPreviousLine, "MoveUpAndModifySelection" },
        { QKeySequence::SelectNextPage, "MovePageDownAndModifySelection" },
        { QKeySequence::SelectPreviousPage, "MovePageUpAndModifySelection" },
        { QKeySequence::SelectStartOfLine, "MoveToBeginningOfLineAndModifySelection" },
        { QKeySequence::SelectEndOfLine, "MoveToEndOfLineAndModifySelection" },
        { QKeySequence::SelectStartOfBlock, "MoveToBeginningOfParagraphAndModifySelection" },
        { QKeySequence::SelectEndOfBlock, "MoveToEndOfParagraphAndModifySelection" },
        { QKeySequence::SelectStartOfDocument, "MoveToBeginningOfDocumentAndModifySelection" },
        { QKeySequence::SelectEndOfDocument, "MoveToEndOfDocumentAndModifySelection" },

        { QKeySequence::DeleteStartOfWord, "DeleteWordBackward" },
        { QKeySequence::DeleteEndOfWord, "DeleteWordForward" },
        { QKeySequence::DeleteEndOfLine, "DeleteToEndOfLine" },
        { QKeySequence::Deselect, "Unselect" },
        { QKeySequence::Backspace, "BackwardDelete" },

        { QKeySequence::UnknownKey, "" }
    };

    for (int i = 0; editCommands[i].standardKey != QKeySequence::UnknownKey; ++i) {
        if (event == editCommands[i].standardKey) {
            *editCommand = editCommands[i].name;
            return true;
        }
    }

#ifdef Q_OS_MACOS
    if ((qtModifiersForEvent(event) & ~Qt::ShiftModifier) == Qt::MetaModifier) {
        if (event->key() == Qt::Key_Backspace) {
            *editCommand = "DeleteToBeginningOfLine";
            return true;
        }
    }
#endif

    return false;
}

} // namespace QtWebEngineCore
