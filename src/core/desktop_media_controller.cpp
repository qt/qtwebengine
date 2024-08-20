// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "desktop_media_controller.h"
#include "desktop_media_controller_p.h"
#include "type_conversion.h"

#include "base/containers/contains.h"
#include "base/functional/callback.h"
#include "chrome/browser/media/webrtc/desktop_capturer_wrapper.h"
#include "chrome/browser/media/webrtc/native_desktop_media_list.h"
#include "content/public/browser/desktop_media_id.h"

#if QT_CONFIG(webengine_webrtc)
#include "content/public/browser/desktop_capture.h"
#endif // QT_CONFIG(webengine_webrtc)

namespace QtWebEngineCore {
namespace {
DesktopMediaList::Type toMediaListType(DesktopMediaType type)
{
    switch (type) {
    case DesktopMediaType::Screen:
        return DesktopMediaList::Type::kScreen;
    case DesktopMediaType::Window:
        return DesktopMediaList::Type::kWindow;
    default:
        return DesktopMediaList::Type::kNone;
    }
}

std::unique_ptr<DesktopMediaList> createMediaList(DesktopMediaType type)
{
#if QT_CONFIG(webengine_webrtc)
    DesktopMediaList::Type listType = toMediaListType(type);
    webrtc::DesktopCaptureOptions options = content::desktop_capture::CreateDesktopCaptureOptions();

    switch (listType) {
    case DesktopMediaList::Type::kScreen: {
        std::unique_ptr<webrtc::DesktopCapturer> screenCapturer =
                webrtc::DesktopCapturer::CreateScreenCapturer(options);
        std::unique_ptr<DesktopCapturerWrapper> capturer =
                std::make_unique<DesktopCapturerWrapper>(std::move(screenCapturer));
        return std::make_unique<NativeDesktopMediaList>(listType, std::move(capturer));
    }
    case DesktopMediaList::Type::kWindow: {
        std::unique_ptr<webrtc::DesktopCapturer> windowCapturer =
                webrtc::DesktopCapturer::CreateWindowCapturer(options);
        std::unique_ptr<DesktopCapturerWrapper> capturer =
                std::make_unique<DesktopCapturerWrapper>(std::move(windowCapturer));
        return std::make_unique<NativeDesktopMediaList>(
                listType, std::move(capturer),
                !content::desktop_capture::ShouldEnumerateCurrentProcessWindows());
    }
    default: {
        Q_UNREACHABLE();
    }
    }
#else
    return nullptr;
#endif // QT_CONFIG(webengine_webrtc)
}
} // namespace

class DesktopMediaListQtPrivate : public DesktopMediaListObserver
{
public:
    DesktopMediaListQtPrivate(DesktopMediaType type, DesktopMediaListQt *qq);

    void init();
    void startUpdating();
    const DesktopMediaList::Source& getSource(int index) const;

    void OnSourceAdded(int index) override;
    void OnSourceRemoved(int index) override;
    void OnSourceMoved(int old_index, int new_index) override;
    void OnSourceNameChanged(int index) override;
    void OnSourceThumbnailChanged(int index) override { }
    void OnSourcePreviewChanged(size_t index) override { }
    void OnDelegatedSourceListSelection() override { }
    void OnDelegatedSourceListDismissed() override { }

    bool isInitialized;
    std::unique_ptr<DesktopMediaList> mediaList;
    DesktopMediaListQt *q_ptr;
    Q_DECLARE_PUBLIC(DesktopMediaListQt)
};

DesktopMediaListQtPrivate::DesktopMediaListQtPrivate(DesktopMediaType type, DesktopMediaListQt *qq)
    : isInitialized(false)
    , mediaList(createMediaList(type))
    , q_ptr(qq)
{
}

const DesktopMediaList::Source& DesktopMediaListQtPrivate::getSource(int index) const
{
    return mediaList->GetSource(index);
}

void DesktopMediaListQtPrivate::init()
{
    // Work around the asynchronous initialization of the source list.
    // DesktopMediaList::Update populates the list and notifies the controller when it completes.
    // This makes direct 'selectScreen/Window' calls possible from the frontend.
    // Note: StartUpdating should be called after Update is completed as it can overwrite the
    // internal cb.
    base::OnceCallback<void()> onComplete = base::BindOnce(
            [](DesktopMediaListQtPrivate *observer) {
                observer->isInitialized = true;
                Q_EMIT observer->q_ptr->initialized();
                observer->startUpdating();
            },
            this);
    mediaList->Update(std::move(onComplete));
}

void DesktopMediaListQtPrivate::startUpdating()
{
    mediaList->StartUpdating(this);
}

void DesktopMediaListQtPrivate::OnSourceAdded(int index)
{
    Q_Q(DesktopMediaListQt);
    Q_EMIT q->sourceAdded(index);
}

void DesktopMediaListQtPrivate::OnSourceRemoved(int index)
{
    Q_Q(DesktopMediaListQt);
    Q_EMIT q->sourceRemoved(index);
}

void DesktopMediaListQtPrivate::OnSourceMoved(int old_index, int new_index)
{
    Q_Q(DesktopMediaListQt);
    Q_EMIT q->sourceMoved(old_index, new_index);
}

void DesktopMediaListQtPrivate::OnSourceNameChanged(int index)
{
    Q_Q(DesktopMediaListQt);
    Q_EMIT q->sourceNameChanged(index);
}

DesktopMediaListQt::DesktopMediaListQt(DesktopMediaType type)
    : d(new DesktopMediaListQtPrivate(type, this))
{
}

DesktopMediaListQt::~DesktopMediaListQt() { }

QString DesktopMediaListQt::getSourceName(int index) const
{
    const auto &source = d->getSource(index);
    return toQt(source.name);
}

int DesktopMediaListQt::getSourceCount() const
{
    return d->mediaList->GetSourceCount();
}

bool DesktopMediaListQt::isInitialized() const
{
    return d->isInitialized;
}

DesktopMediaControllerPrivate::DesktopMediaControllerPrivate(
        base::OnceCallback<void(content::DesktopMediaID)> doneCallback)
    : doneCallback(std::move(doneCallback))
    , screens(new DesktopMediaListQt(DesktopMediaType::Screen))
    , windows(new DesktopMediaListQt(DesktopMediaType::Window))
{
}

void DesktopMediaControllerPrivate::selectScreen(int index)
{
    const auto &source = screens->d->getSource(index);
    std::move(doneCallback).Run(source.id);
}

void DesktopMediaControllerPrivate::selectWindow(int index)
{
    const auto &source = windows->d->getSource(index);
    std::move(doneCallback).Run(source.id);
}

void DesktopMediaControllerPrivate::cancel()
{
    std::move(doneCallback).Run({});
}

DesktopMediaController::DesktopMediaController(DesktopMediaControllerPrivate *dd)
    : d(dd)
{
    // Make sure both lists are populated before sending the request.
    DesktopMediaListQt *screens = DesktopMediaController::screens();
    DesktopMediaListQt *windows = DesktopMediaController::windows();
    QObject::connect(screens, &DesktopMediaListQt::initialized, [windows, this]() {
        if (windows->isInitialized())
            Q_EMIT mediaListsInitialized();
    });

    QObject::connect(windows, &DesktopMediaListQt::initialized, [screens, this]() {
        if (screens->isInitialized())
            Q_EMIT mediaListsInitialized();
    });

    screens->d->init();
    windows->d->init();
}

DesktopMediaController::~DesktopMediaController()
{
}

void DesktopMediaController::selectScreen(int index)
{
    d->selectScreen(index);
}

void DesktopMediaController::selectWindow(int index)
{
    d->selectWindow(index);
}

void DesktopMediaController::cancel()
{
    d->cancel();
}

DesktopMediaListQt *DesktopMediaController::screens() const
{
    return d->screens.data();
}

DesktopMediaListQt *DesktopMediaController::windows() const
{
    return d->windows.data();
}

} // namespace QtWebEngineCore
