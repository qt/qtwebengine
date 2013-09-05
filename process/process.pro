# This is a dummy .pro file used to extract some aspects of the used configuration and feed them to gyp
# We want the gyp generation step to happen after all the other config steps. For that we need to prepend
# our gyp_generator.prf feature to the CONFIG variable since it is processed backwards
CONFIG = gyp_generator $$CONFIG
GYPDEPENDENCIES += ../shared/shared.gyp:qtwebengine_shared
GYPINCLUDES += ../qtwebengine.gypi

TARGET = $$QTWEBENGINEPROCESS_NAME
TEMPLATE = app

SOURCES = main.cpp \
         ../3rdparty/chromium/content/browser/aura/image_transport_factory.cc \
        ../3rdparty/chromium/content/browser/aura/no_transport_image_transport_factory.cc \
        ../3rdparty/chromium/content/browser/aura/gpu_process_transport_factory.cc \
        ../3rdparty/chromium/content/browser/aura/browser_compositor_output_surface.cc \
        ../3rdparty/chromium/content/browser/aura/browser_compositor_output_surface_proxy.cc \
        ../3rdparty/chromium/content/browser/aura/software_output_device_x11.cc \
        ../3rdparty/chromium/content/browser/aura/reflector_impl.cc \
        ../3rdparty/chromium/content/browser/aura/software_browser_compositor_output_surface.cc \
        ../3rdparty/chromium/ui/compositor/compositor_switches.cc \
        ../3rdparty/chromium/ui/compositor/compositor.cc \
        ../3rdparty/chromium/ui/compositor/layer.cc \
        ../3rdparty/chromium/ui/compositor/layer_animator.cc \
        ../3rdparty/chromium/ui/compositor/dip_util.cc \
        ../3rdparty/chromium/ui/compositor/layer_animation_sequence.cc \
        ../3rdparty/chromium/ui/compositor/layer_animation_element.cc \
        ../3rdparty/chromium/ui/compositor/layer_animation_observer.cc \
        ../3rdparty/chromium/ui/compositor/float_animation_curve_adapter.cc \
        ../3rdparty/chromium/ui/compositor/transform_animation_curve_adapter.cc \
        ../3rdparty/chromium/ui/compositor/scoped_animation_duration_scale_mode.cc \
        ../3rdparty/chromium/ui/compositor/layer_owner.cc


