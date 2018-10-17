/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef LOCKED_PTR_H
#define LOCKED_PTR_H

#include <base/bind_internal.h>

#include <QtCore/qreadwritelock.h>

namespace base {

struct LockedPtrCore
{
    LockedPtrCore(uintptr_t data) : data(data) {}

    std::atomic<size_t> refCount{1};
    // Atomic so that WeakLockedPtr::get can still read it.
    std::atomic<uintptr_t> data;
    QReadWriteLock lock{QReadWriteLock::Recursive};
};

enum class LockedPtrMode { Weak, Shared, Exclusive };

template<class T, LockedPtrMode mode> class LockedPtr;

// A WeakLockedPtr<T> is something like shared_ptr<T*>. The T* value can only be
// accessed by atomic read.
template<class T> using WeakLockedPtr = LockedPtr<T, LockedPtrMode::Weak>;

// A SharedLockedPtr<T> is like WeakLockedPtr<T>, but the T* value is prevented
// from changing for the lifetime of the SharedLockedPtr by holding a
// shared-exclusive mutex in shared mode.
template<class T> using SharedLockedPtr = LockedPtr<T, LockedPtrMode::Shared>;

// An ExclusiveLockedPtr<T> is like SharedLockedPtr<T>, but the mutex is held in
// exclusive mode. Only in this mode can the T* value be changed.
template<class T> using ExclusiveLockedPtr = LockedPtr<T, LockedPtrMode::Exclusive>;

template<class T, LockedPtrMode mode>
class LockedPtr
{
    template<class T1>
    static constexpr bool canConstructFrom =
        std::is_same<T, T1>::value ||
        std::is_same<T, const T1>::value;

public:
    constexpr LockedPtr() {}
    constexpr LockedPtr(std::nullptr_t) {}

    LockedPtr(const LockedPtr &that)
    {
        m_core = that.m_core;
        lock();
    }

    LockedPtr &operator=(const LockedPtr &that)
    {
        unlock();
        m_core = that.m_core;
        lock();
    }

    LockedPtr(LockedPtr &&that)
    {
        m_core = that.m_core;
        that.m_core = nullptr;
    }

    LockedPtr &operator=(LockedPtr &&that)
    {
        unlock();
        m_core = that.m_core;
        that.m_core = nullptr;
    }

    template<class T1, LockedPtrMode mode1,
             class Enable = std::enable_if_t<canConstructFrom<T1>>>
    LockedPtr(const LockedPtr<T1, mode1> &that)
    {
        m_core = that.m_core;
        lock();
    }

    template<class T1, LockedPtrMode mode1,
             class Enable = std::enable_if_t<canConstructFrom<T1>>>
    LockedPtr &operator=(const LockedPtr<T1, mode1> &that)
    {
        unlock();
        m_core = that.m_core;
        lock();
    }

    template<class T1,
             class Enable = std::enable_if_t<canConstructFrom<T1>>>
    LockedPtr(LockedPtr<T1, mode> &&that)
    {
        m_core = that.m_core;
        that.m_core = nullptr;
    }

    template<class T1,
             class Enable = std::enable_if_t<canConstructFrom<T1>>>
    LockedPtr &operator=(LockedPtr<T1, mode> &&that)
    {
        unlock();
        m_core = that.m_core;
        that.m_core = nullptr;
    }

    ~LockedPtr()
    {
        unlock();
    }

    T *get() const
    {
        if (m_core) {
            if (mode == LockedPtrMode::Weak)
                return reinterpret_cast<T *>(m_core->data.load(std::memory_order_acquire));
            else
                return reinterpret_cast<T *>(m_core->data.load(std::memory_order_relaxed));
        }
        return nullptr;
    }

    void set(T *value)
    {
        static_assert(mode == LockedPtrMode::Exclusive, "");
        DCHECK(m_core);
        m_core->data.store(reinterpret_cast<uintptr_t>(value), std::memory_order_release);
    }

    T &operator*() const { return *get(); }
    T *operator->() const { return get(); }
    explicit operator bool() const { return get(); }

    bool MaybeValid() const { return m_core; }

    static LockedPtr create(T *value)
    {
        return new LockedPtrCore(reinterpret_cast<uintptr_t>(value));
    }

private:
    template<class T1, LockedPtrMode mode1> friend class LockedPtr;

    LockedPtr(LockedPtrCore *core)
        : m_core(core)
    {}

    void lock()
    {
        if (m_core) {
            ++m_core->refCount;

            if (mode == LockedPtrMode::Shared)
                m_core->lock.lockForRead();
            else if (mode == LockedPtrMode::Exclusive)
                m_core->lock.lockForWrite();
        }
    }

    void unlock()
    {
        if (m_core) {
            if (mode != LockedPtrMode::Weak)
                m_core->lock.unlock();

            if (--m_core->refCount == 0)
                delete m_core;
        }
    }

    LockedPtrCore *m_core = nullptr;
};

// This makes Bind check the pointer before calling the functor.
template<class T>
struct IsWeakReceiver<WeakLockedPtr<T>> : std::true_type {};

// By converting the WeakLockedPtr into a SharedLockedPtr we prevent the
// pointed-to object from being destroyed during the base::Callback::Run call.
//
// Unwrap() is called before checking the pointer, so there's no race condition.
template<class T>
struct BindUnwrapTraits<WeakLockedPtr<T>>
{
    static SharedLockedPtr<T> Unwrap(const WeakLockedPtr<T> &o)
    {
        return o;
    }
};

// Like base::WeakPtrFactory, but InvalidateWeakPtrs *waits* until all currently
// executing base::Callbacks are finished. Queued up base::Callbacks are still
// canceled, exactly like with WeakPtrFactory.
//
// Consider, for example, the function
//
//     void fun()
//     {
//         MyClass *myClass = new MyClass;
//         myClass->scheduleDoStuff();
//         delete myClass; // ???
//     }
//
// where
//
//     class MyClass
//     {
//     public:
//         void scheduleDoStuff()
//         {
//             content::BrowserThread::PostTask(
//                 content::BrowserThread::IO, FROM_HERE,
//                 base::BindOnce(&MyClass::doStuff, m_weakPtrFactory.GetWeakPtr()));
//         }
//         void doStuff();
//     private:
//         //base::WeakPtrFactory m_weakPtrFactory{this};
//         base::LockedPtrFactory m_weakPtrFactory{this};
//     };
//
// What happens if the 'delete myClass' line is executed concurrently with
// MyClass::doStuff?
//
// With WeakPtrs we get a segfault or perhaps memory corruption.
//
// With LockedPtrs we get no crash and no corruption: LockedPtrFactory's
// destructor will wait until doStuff is done before continuing.
template<class T>
class LockedPtrFactory
{
public:
    explicit LockedPtrFactory(T *value)
        : m_ptr(WeakLockedPtr<T>::create(value))
    {}

    ~LockedPtrFactory()
    {
        InvalidateWeakPtrs();
    }

    WeakLockedPtr<T> GetWeakPtr() { return m_ptr; }
    WeakLockedPtr<const T> GetWeakPtr() const { return m_ptr; }
    SharedLockedPtr<T> GetSharedPtr() { return m_ptr; }
    SharedLockedPtr<const T> GetSharedPtr() const { return m_ptr; }
    ExclusiveLockedPtr<T> GetExclusivePtr() { return m_ptr; }
    ExclusiveLockedPtr<const T> GetExclusivePtr() const { return m_ptr; }

    void InvalidateWeakPtrs()
    {
        if (ExclusiveLockedPtr<T> ptr = m_ptr)
            ptr.set(nullptr);
    }

private:
    WeakLockedPtr<T> m_ptr;
};

} // namespace base

#endif // !LOCKED_PTR_H
