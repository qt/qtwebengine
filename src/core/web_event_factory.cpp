/****************************************************************************
**
** Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#include "web_event_factory.h"
#include "third_party/WebKit/Source/platform/WindowsKeyboardCodes.h"

#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

static const int wheelScrollLines = 3; // FIXME: Still not available in QStyleHints in 5.1

using namespace blink;

static int windowsKeyCodeForKeyEvent(unsigned int keycode, bool isKeypad)
{
    // Determine wheter the event comes from the keypad
    if (isKeypad) {
        switch (keycode) {
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
        default:
            return 0;
        }

    } else

    switch (keycode) {
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
    case Qt::Key_Menu:
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
    case Qt::Key_ParenLeft:
        return VK_0; // (30) 0) key
    case Qt::Key_1:
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
    case Qt::Key_ParenRight:
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
        // VK_APPS (5D) Applications key (Natural keyboard)
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
        // VK_VOLUME_MUTE (AD) Windows 2000/XP: Volume Mute key
        // VK_VOLUME_DOWN (AE) Windows 2000/XP: Volume Down key
        // VK_VOLUME_UP (AF) Windows 2000/XP: Volume Up key
        // VK_MEDIA_NEXT_TRACK (B0) Windows 2000/XP: Next Track key
        // VK_MEDIA_PREV_TRACK (B1) Windows 2000/XP: Previous Track key
        // VK_MEDIA_STOP (B2) Windows 2000/XP: Stop Media key
        // VK_MEDIA_PLAY_PAUSE (B3) Windows 2000/XP: Play/Pause Media key
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
    case Qt::Key_QuoteDbl:
        return VK_OEM_7; // case '\'': case '"': return 0xDE;
        // VK_OEM_8 (DF) Used for miscellaneous characters; it can vary by keyboard.
        // VK_OEM_102 (E2) Windows 2000/XP: Either the angle bracket key or the backslash key on the RT 102-key keyboard
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

static inline double currentTimeForEvent(const QInputEvent* event)
{
    Q_ASSERT(event);

    if (event->timestamp())
        return static_cast<double>(event->timestamp()) / 1000;

    static QElapsedTimer timer;
    if (!timer.isValid())
        timer.start();
    return static_cast<double>(timer.elapsed()) / 1000;
}

static WebMouseEvent::Button mouseButtonForEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || (event->buttons() & Qt::LeftButton))
        return WebMouseEvent::ButtonLeft;
    else if (event->button() == Qt::RightButton || (event->buttons() & Qt::RightButton))
        return WebMouseEvent::ButtonRight;
    else if (event->button() == Qt::MidButton || (event->buttons() & Qt::MidButton))
        return WebMouseEvent::ButtonMiddle;
    return WebMouseEvent::ButtonNone;
}

template <typename T>
static unsigned mouseButtonsModifiersForEvent(const T* event)
{
    unsigned ret = 0;
    if (event->buttons() & Qt::LeftButton)
        ret |= WebInputEvent::LeftButtonDown;
    if (event->buttons() & Qt::RightButton)
        ret |= WebInputEvent::RightButtonDown;
    if (event->buttons() & Qt::MidButton)
        ret |= WebInputEvent::MiddleButtonDown;
    return ret;
}

static inline WebInputEvent::Modifiers modifiersForEvent(const QInputEvent* event)
{
    unsigned result = 0;
    Qt::KeyboardModifiers modifiers = event->modifiers();
    if (modifiers & Qt::ShiftModifier)
        result |= WebInputEvent::ShiftKey;
    if (modifiers & Qt::ControlModifier)
        result |= WebInputEvent::ControlKey;
    if (modifiers & Qt::AltModifier)
        result |= WebInputEvent::AltKey;
    if (modifiers & Qt::MetaModifier)
        result |= WebInputEvent::MetaKey;
    if (modifiers & Qt::KeypadModifier)
        result |= WebInputEvent::IsKeyPad;

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
    case QEvent::KeyRelease:
        if (static_cast<const QKeyEvent*>(event)->isAutoRepeat())
            result |= WebInputEvent::IsAutoRepeat;
    default:
        break;
    }

    return (WebInputEvent::Modifiers)result;
}

static WebInputEvent::Type webEventTypeForEvent(const QEvent* event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        return WebInputEvent::MouseDown;
    case QEvent::MouseButtonRelease:
        return WebInputEvent::MouseUp;
    case QEvent::Enter:
        return WebInputEvent::MouseEnter;
    case QEvent::Leave:
        return WebInputEvent::MouseLeave;
    case QEvent::MouseMove:
        return WebInputEvent::MouseMove;
    case QEvent::Wheel:
        return WebInputEvent::MouseWheel;
    case QEvent::KeyPress:
        return WebInputEvent::RawKeyDown;
    case QEvent::KeyRelease:
        return WebInputEvent::KeyUp;
    case QEvent::HoverMove:
        return WebInputEvent::MouseMove;
    case QEvent::TouchBegin:
        return WebInputEvent::TouchStart;
    case QEvent::TouchUpdate:
        return WebInputEvent::TouchMove;
    case QEvent::TouchEnd:
        return WebInputEvent::TouchEnd;
    case QEvent::TouchCancel:
        return WebInputEvent::TouchCancel;
    default:
        Q_ASSERT(false);
        return WebInputEvent::MouseMove;
    }
}

WebMouseEvent WebEventFactory::toWebMouseEvent(QMouseEvent *ev, double dpiScale)
{
    WebMouseEvent webKitEvent;
    webKitEvent.timeStampSeconds = currentTimeForEvent(ev);
    webKitEvent.button = mouseButtonForEvent(ev);
    webKitEvent.modifiers = modifiersForEvent(ev);

    webKitEvent.x = webKitEvent.windowX = ev->x() / dpiScale;
    webKitEvent.y = webKitEvent.windowY = ev->y() / dpiScale;
    webKitEvent.globalX = ev->globalX();
    webKitEvent.globalY = ev->globalY();

    webKitEvent.type = webEventTypeForEvent(ev);
    webKitEvent.clickCount = 0;

    return webKitEvent;
}

WebMouseEvent WebEventFactory::toWebMouseEvent(QHoverEvent *ev, double dpiScale)
{
    WebMouseEvent webKitEvent;
    webKitEvent.timeStampSeconds = currentTimeForEvent(ev);
    webKitEvent.modifiers = modifiersForEvent(ev);

    webKitEvent.x = webKitEvent.windowX = ev->pos().x() / dpiScale;
    webKitEvent.y = webKitEvent.windowY = ev->pos().y() / dpiScale;

    webKitEvent.type = webEventTypeForEvent(ev);
    return webKitEvent;
}

blink::WebMouseWheelEvent WebEventFactory::toWebWheelEvent(QWheelEvent *ev, double dpiScale)
{
    WebMouseWheelEvent webEvent;
    webEvent.type = webEventTypeForEvent(ev);
    webEvent.deltaX = 0;
    webEvent.deltaY = 0;
    webEvent.wheelTicksX = 0;
    webEvent.wheelTicksY = 0;
    webEvent.modifiers = modifiersForEvent(ev);
    webEvent.timeStampSeconds = currentTimeForEvent(ev);

    if (ev->orientation() == Qt::Horizontal)
        webEvent.wheelTicksX = ev->delta() / 120.0f;
    else
        webEvent.wheelTicksY = ev->delta() / 120.0f;


    // Since we report the scroll by the pixel, convert the delta to pixel distance using standard scroll step.
    // Use the same single scroll step as QTextEdit (in QTextEditPrivate::init [h,v]bar->setSingleStep)
    static const float cDefaultQtScrollStep = 20.f;

    webEvent.deltaX = webEvent.wheelTicksX * wheelScrollLines * cDefaultQtScrollStep;
    webEvent.deltaY = webEvent.wheelTicksY * wheelScrollLines * cDefaultQtScrollStep;

    webEvent.x = webEvent.windowX = ev->x() / dpiScale;
    webEvent.y = webEvent.windowY = ev->y() / dpiScale;
    webEvent.globalX = ev->globalX();
    webEvent.globalY = ev->globalY();
    return webEvent;
}

content::NativeWebKeyboardEvent WebEventFactory::toWebKeyboardEvent(QKeyEvent *ev)
{
    content::NativeWebKeyboardEvent webKitEvent;
    webKitEvent.timeStampSeconds = currentTimeForEvent(ev);
    webKitEvent.modifiers = modifiersForEvent(ev);
    webKitEvent.type = webEventTypeForEvent(ev);

    webKitEvent.nativeKeyCode = ev->nativeVirtualKey();
    webKitEvent.windowsKeyCode = windowsKeyCodeForKeyEvent(ev->key(), ev->modifiers() & Qt::KeypadModifier);
    webKitEvent.setKeyIdentifierFromWindowsKeyCode();

    const ushort* text = ev->text().utf16();
    memcpy(&webKitEvent.text, text, std::min(sizeof(webKitEvent.text), size_t(ev->text().length() * 2)));
    memcpy(&webKitEvent.unmodifiedText, text, std::min(sizeof(webKitEvent.unmodifiedText), size_t(ev->text().length() * 2)));
    return webKitEvent;
}
