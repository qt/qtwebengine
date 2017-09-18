/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef WAITFORSIGNAL_H
#define WAITFORSIGNAL_H

#include <QObject>
#include <QTestEventLoop>

#include <utility>

// Implementation details of waitForSignal.
namespace {
    // Wraps a functor to set a flag and exit from event loop if called.
    template <class SignalHandler>
    struct waitForSignal_SignalHandlerWrapper {
        waitForSignal_SignalHandlerWrapper(SignalHandler &&sh)
            : signalHandler(std::forward<SignalHandler>(sh)) {}

        template <class... Args>
        void operator()(Args && ... args) {
            signalHandler(std::forward<Args>(args)...);
            hasBeenCalled = true;
            loop.exitLoop();
        }

        SignalHandler &&signalHandler;
        QTestEventLoop loop;
        bool hasBeenCalled = false;
    };

    // No-op functor.
    struct waitForSignal_DefaultSignalHandler {
        template <class... Args>
        void operator()(Args && ...) {}
    };
} // namespace

// Wait for a signal to be emitted.
//
// The given functor is called with the signal arguments allowing the arguments
// to be modified before returning from the signal handler (unlike QSignalSpy).
template <class Sender, class Signal, class SignalHandler>
bool waitForSignal(Sender &&sender, Signal &&signal, SignalHandler &&signalHandler, int timeout = 15000)
{
    waitForSignal_SignalHandlerWrapper<SignalHandler> signalHandlerWrapper(
        std::forward<SignalHandler>(signalHandler));
    auto connection = QObject::connect(
        std::forward<Sender>(sender),
        std::forward<Signal>(signal),
        std::ref(signalHandlerWrapper));
    signalHandlerWrapper.loop.enterLoopMSecs(timeout);
    QObject::disconnect(connection);
    return signalHandlerWrapper.hasBeenCalled;
}

template <class Sender, class Signal>
bool waitForSignal(Sender &&sender, Signal &&signal, int timeout = 15000)
{
    return waitForSignal(std::forward<Sender>(sender),
                         std::forward<Signal>(signal),
                         waitForSignal_DefaultSignalHandler(),
                         timeout);
}

#endif // !WAITFORSIGNAL_H
