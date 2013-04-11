CHROMIUM_SRC_DIR = $$(CHROMIUM_SRC_DIR)
isEmpty(CHROMIUM_SRC_DIR): error("Set CHROMIUM_SRC_DIR please...")

INCLUDEPATH += $$CHROMIUM_SRC_DIR/

CONFIG(debug, debug|release): BUILD_DIR = $$CHROMIUM_SRC_DIR/out/Debug
else: BUILD_DIR = $$CHROMIUM_SRC_DIR/out/Release

exists($$BUILD_DIR/obj/content/libcontent_app.a): CONFIG += chromium_is_static

chromium_is_static {
    cr_libs = \
        obj/media/libmedia_sse.a \
        obj/third_party/icu/libicuuc.a \
        obj/third_party/libjingle/libjingle.a \
        obj/skia/libskia_opts.a \
        obj/third_party/icu/libicudata.a \
        obj/third_party/webrtc/modules/librtp_rtcp.a \
        obj/third_party/libvpx/libvpx.a \
        obj/webkit/support/libglue.a \
        obj/sandbox/libseccomp_bpf.a \
        obj/third_party/libwebp/libwebp_utils.a \
        obj/third_party/WebKit/Source/WTF/WTF.gyp/libwtf.a \
        obj/third_party/webrtc/modules/libG722.a \
        obj/third_party/libvpx/libvpx_intrinsics.a \
        obj/third_party/webrtc/modules/libCNG.a \
        obj/third_party/WebKit/Source/WebCore/WebCore.gyp/libwebcore_rendering.a \
        obj/ppapi/libppapi_unittest_shared.a \
        obj/media/libyuv_convert_simd_x86.a \
        obj/gpu/libgles2_cmd_helper.a \
        obj/third_party/smhasher/libcityhash.a \
        obj/third_party/webrtc/modules/libudp_transport.a \
        obj/third_party/webrtc/common_audio/libresampler.a \
        obj/content/libcontent_gpu.a \
        obj/ipc/libipc.a \
        obj/third_party/libxslt/libxslt.a \
        obj/third_party/hyphen/libhyphen.a \
        obj/third_party/webrtc/modules/video_coding/utility/libvideo_coding_utility.a \
        obj/third_party/ots/libots.a \
        obj/base/libsymbolize.a \
        obj/skia/libskia_opts_ssse3.a \
        obj/third_party/protobuf/libprotobuf_lite.a \
        obj/third_party/webrtc/modules/libaudio_coding_module.a \
        obj/ui/surface/libsurface.a \
        obj/third_party/WebKit/Source/WebKit/chromium/libwebkit.a \
        obj/third_party/jsoncpp/libjsoncpp.a \
        obj/google_apis/libgoogle_apis.a \
        obj/webkit/gpu/libwebkit_gpu.a \
        obj/third_party/WebKit/Source/WebKit/chromium/libwebkit_wtf_support.a \
        obj/v8/tools/gyp/libv8_snapshot.a \
        obj/ui/native_theme/libnative_theme.a \
        obj/third_party/webrtc/modules/libremote_bitrate_estimator.a \
        obj/content/libcontent_utility.a \
        obj/base/libbase_static.a \
        obj/third_party/webrtc/modules/libwebrtc_video_coding.a \
        obj/webkit/compositor_bindings/libwebkit_compositor_support.a \
        obj/third_party/libevent/libevent.a \
        obj/content/libcontent_worker.a \
        obj/build/linux/libpci.a \
        obj/ui/gl/libgl_wrapper.a \
        obj/third_party/angle/src/libtranslator_common.a \
        obj/third_party/WebKit/Source/WebKit/chromium/libwebkit_test_support.a \
        obj/content/libcontent_shell_lib.a \
        obj/third_party/libpng/libpng.a \
        obj/third_party/webrtc/modules/libG711.a \
        obj/third_party/opus/libopus.a \
        obj/content/libcontent_ppapi_plugin.a \
        obj/ui/snapshot/libsnapshot.a \
        obj/webkit/support/libwebkit_media.a \
        obj/third_party/libwebp/libwebp_dsp.a \
        obj/third_party/harfbuzz-ng/libharfbuzz-ng.a \
        obj/base/allocator/liballocator_extension_thunks.a \
        obj/third_party/libjingle/libjingle_webrtc.a \
        obj/third_party/webrtc/voice_engine/libvoice_engine_core.a \
        obj/cc/libcc.a \
        obj/third_party/libjingle/libjingle_p2p_constants.a \
        obj/media/libyuv_convert.a \
        obj/third_party/leveldatabase/libleveldatabase.a \
        obj/net/libnet_test_support.a \
        obj/third_party/webrtc/modules/libiLBC.a \
        obj/third_party/webrtc/modules/libpaced_sender.a \
        obj/gpu/libdisk_cache_proto.a \
        obj/sandbox/libsuid_sandbox_client.a \
        obj/third_party/webrtc/modules/libiSACFix.a \
        obj/third_party/webrtc/modules/libvideo_capture_module.a \
        obj/third_party/angle/src/libpreprocessor.a \
        obj/gpu/libgpu_ipc.a \
        obj/third_party/webrtc/modules/video_coding/codecs/vp8/libwebrtc_vp8.a \
        obj/ppapi/libppapi_host.a \
        obj/third_party/WebKit/Source/ThirdParty/glu/libtess.a \
        obj/dbus/libdbus.a \
        obj/third_party/webrtc/modules/libvideo_processing.a \
        obj/webkit/support/libwebkit_base.a \
        obj/gpu/libcommand_buffer_client.a \
        obj/gpu/libcommand_buffer_common.a \
        obj/third_party/libwebp/libwebp_enc.a \
        obj/third_party/WebKit/Source/WebCore/WebCore.gyp/libwebcore_html.a \
        obj/base/libtest_support_base.a \
        obj/ppapi/libppapi_ipc.a \
        obj/sdch/libsdch.a \
        obj/third_party/angle/src/libtranslator_glsl.a \
        obj/ppapi/libppapi_shared.a \
        obj/testing/libgmock.a \
        obj/gpu/libgles2_implementation.a \
        obj/sandbox/libc_urandom_override.a \
        obj/third_party/libXNVCtrl/libXNVCtrl.a \
        obj/content/browser/speech/proto/libspeech_proto.a \
        obj/third_party/webrtc/modules/libvideo_render_module.a \
        obj/build/temp_gyp/libgoogleurl.a \
        obj/webkit/support/libuser_agent.a \
        obj/ui/libui.a \
        obj/testing/libgtest.a \
        obj/third_party/webrtc/modules/libwebrtc_opus.a \
        obj/base/libxdg_mime.a \
        obj/content/libcontent_browser.a \
        obj/third_party/webrtc/modules/libNetEq.a \
        obj/third_party/webrtc/video_engine/libvideo_engine_core.a \
        obj/base/third_party/dynamic_annotations/libdynamic_annotations.a \
        obj/third_party/WebKit/Source/WebCore/WebCore.gyp/libwebcore_remaining.a \
        obj/third_party/sfntly/libsfntly.a \
        obj/base/libbase.a \
        obj/third_party/WebKit/Source/WebCore/WebCore.gyp/libwebcore_svg.a \
        obj/crypto/libcrcrypto.a \
        obj/third_party/WebKit/Source/WebCore/WebCore.gyp/libwebcore_dom.a \
        obj/media/libshared_memory_support.a \
        obj/third_party/qcms/libqcms.a \
        obj/third_party/webrtc/modules/libPCM16B.a \
        obj/ui/libshell_dialogs.a \
        obj/third_party/webrtc/modules/libvideo_processing_sse2.a \
        obj/third_party/webrtc/modules/libbitrate_controller.a \
        obj/sandbox/libsandbox_services.a \
        obj/third_party/libwebp/libwebp_demux.a \
        obj/third_party/sqlite/libsqlite3.a \
        obj/third_party/webrtc/modules/libmedia_file.a \
        obj/third_party/libjpeg_turbo/libjpeg_turbo.a \
        obj/third_party/modp_b64/libmodp_b64.a \
        obj/third_party/libvpx/libvpx_asm_offsets_vp9.a \
        obj/third_party/webrtc/modules/libaudio_processing.a \
        obj/ipc/libtest_support_ipc.a \
        obj/build/linux/libgio.a \
        obj/third_party/flac/libflac.a \
        obj/third_party/speex/libspeex.a \
        obj/third_party/re2/libre2.a \
        obj/net/third_party/nss/libcrssl.a \
        obj/jingle/libjingle_glue.a \
        obj/third_party/libyuv/libyuv.a \
        obj/third_party/libjingle/libpeerconnection.a \
        obj/content/libcontent_common.a \
        obj/base/libbase_i18n.a \
        obj/third_party/libxml/libxml2.a \
        obj/third_party/WebKit/Source/WebCore/WebCore.gyp/libwebcore_bindings.a \
        obj/ppapi/libppapi_proxy.a \
        obj/third_party/libwebp/libwebp_dec.a \
        obj/third_party/webrtc/modules/libiSAC.a \
        obj/gpu/libgles2_c_lib.a \
        obj/third_party/webrtc/modules/libaudio_conference_mixer.a \
        obj/net/libhttp_server.a \
        obj/sandbox/linux/seccomp-legacy/libseccomp_sandbox.a \
        obj/webkit/support/libwebkit_support_common.a \
        obj/third_party/WebKit/Source/WebCore/WebCore.gyp/libwebcore_test_support.a \
        obj/content/libcontent_app.a \
        obj/third_party/webrtc/common_video/libcommon_video.a \
        obj/third_party/libsrtp/libsrtp.a \
        obj/printing/libprinting.a \
        obj/content/libcontent_renderer.a \
        obj/third_party/WebKit/Tools/DumpRenderTree/DumpRenderTree.gyp/libTestRunner.a \
        obj/third_party/webrtc/modules/libwebrtc_i420.a \
        obj/third_party/iccjpeg/libiccjpeg.a \
        obj/third_party/icu/libicui18n.a \
        obj/webkit/support/libwebkit_support.a \
        obj/third_party/WebKit/Source/Platform/Platform.gyp/libwebkit_platform.a \
        obj/content/libcontent_plugin.a \
        obj/third_party/webrtc/modules/libaudio_device.a \
        obj/third_party/WebKit/Source/WebCore/WebCore.gyp/libwebcore_platform_geometry.a \
        obj/skia/libskia.a \
        obj/webkit/support/libwebkit_storage.a \
        obj/webkit/compositor_bindings/libwebkit_compositor_bindings.a \
        obj/third_party/WebKit/Source/yarr/libyarr.a \
        obj/base/allocator/liballocator.a \
        obj/media/libdiffer_block_sse2.a \
        obj/third_party/webrtc/system_wrappers/source/libsystem_wrappers.a \
        obj/third_party/webrtc/common_audio/libsignal_processing.a \
        obj/third_party/webrtc/modules/libaudio_processing_sse2.a \
        obj/third_party/libvpx/libvpx_asm_offsets.a \
        obj/third_party/WebKit/Source/WebCore/WebCore.gyp/libwebcore_platform.a \
        obj/ui/libui_test_support.a \
        obj/gpu/command_buffer/libgles2_utils.a \
        obj/net/libnet.a \
        obj/v8/tools/gyp/libv8_base.a \
        obj/webkit/support/libwebkit_support_gfx.a \
        obj/content/libtest_support_content.a \
        obj/third_party/webrtc/common_audio/libvad.a \
        obj/net/libnet_with_v8.a \
        obj/third_party/ffmpeg/libffmpeg.a \
        obj/gpu/libcommand_buffer_service.a \
        obj/third_party/webrtc/modules/libwebrtc_utility.a \
        obj/third_party/v8-i18n/build/libv8-i18n.a \
        obj/sql/libsql.a \
        obj/media/libmedia.a \
        obj/third_party/zlib/libchrome_zlib.a \
        obj/components/libtracing.a

    LIBS += -Wl,--start-group
    for (l, cr_libs) {
        LIBS += $$BUILD_DIR/$$l
    }
    LIBS += -Wl,--end-group

    cr_sh_libs = -lX11 -lXcursor -lXrandr -lXrender -lXcomposite -lrt -ldl \
        -lgmodule-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0 -lXtst \
        -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 \
        -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo -lpango-1.0 -lfreetype \
        -lfontconfig -lXi -lasound -lXdamage -lXext -lXfixes -lnss3 -lnssutil3 \
        -lsmime3 -lplds4 -lplc4 -lnspr4 -lgconf-2 -lresolv -ldbus-1 -lcups \
        -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err -lgnutls -lgcrypt -lz \
        -lpthread -lm -lcrypt -L/lib/x86_64-linux-gnu -lexpat -ludev

    LIBS += $$cr_sh_libs

} else {
    DIRS = lib
    CONTENT_LIB = -lcontent
}

for (dir, DIRS) {
    QMAKE_LIBDIR += $$BUILD_DIR/$$dir
}

QMAKE_CXXFLAGS += -D_FILE_OFFSET_BITS=64 -DUSE_LINUX_BREAKPAD -DDISABLE_NACL -DCHROMIUM_BUILD -DUSE_DEFAULT_RENDER_THEME=1 -DUSE_LIBJPEG_TURBO=1 -DUSE_NSS=1 -DENABLE_ONE_CLICK_SIGNIN -DGTK_DISABLE_SINGLE_INCLUDES=1 -DENABLE_REMOTING=1 -DENABLE_WEBRTC=1 -DENABLE_CONFIGURATION_POLICY -DENABLE_INPUT_SPEECH -DENABLE_NOTIFICATIONS -DENABLE_GPU=1 -DENABLE_EGLIMAGE=1 -DENABLE_TASK_MANAGER=1 -DENABLE_EXTENSIONS=1 -DENABLE_PLUGIN_INSTALLATION=1 -DENABLE_PLUGINS=1 -DENABLE_SESSION_SERVICE=1 -DENABLE_THEMES=1 -DENABLE_BACKGROUND=1 -DENABLE_AUTOMATION=1 -DENABLE_GOOGLE_NOW=1 -DENABLE_LANGUAGE_DETECTION=1 -DENABLE_PRINTING=1 -DENABLE_CAPTIVE_PORTAL_DETECTION=1 -DENABLE_MANAGED_USERS=1 '-DCONTENT_SHELL_VERSION="19.77.34.5"' -DGL_GLEXT_PROTOTYPES -DLIBPEERCONNECTION_LIB=1 -DSK_BUILD_NO_IMAGE_ENCODE -DSK_DEFERRED_CANVAS_USES_GPIPE=1 '-DGR_GL_CUSTOM_SETUP_HEADER="GrGLConfig_chrome.h"' -DGR_AGGRESSIVE_SHADER_OPTS=1 -DSK_ENABLE_INST_COUNT=0 -DSK_USE_POSIX_THREADS -DU_USING_ICU_NAMESPACE=0 -DU_STATIC_IMPLEMENTATION -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -DNVALGRIND -DDYNAMIC_ANNOTATIONS_ENABLED=0 -D_FORTIFY_SOURCE=2

cr_inc_paths += \
    ../../third_party/icu/public/common \
    ../../third_party/icu/public/i18n \
    ../.. -I../../third_party/khronos \
    ../../gpu \
    gen/content \
    gen/net \
    ../../skia/config \
    ../../third_party/skia/src/core \
    ../../third_party/skia/include/config \
    ../../third_party/skia/include/core \
    ../../third_party/skia/include/effects \
    ../../third_party/skia/include/pdf \
    ../../third_party/skia/include/gpu \
    ../../third_party/skia/include/gpu/gl \
    ../../third_party/skia/include/pipe \
    ../../third_party/skia/include/ports \
    ../../third_party/skia/include/utils \
    ../../skia/ext \
    gen/ui/gl \
    ../../third_party/mesa/MesaLib/include \
    ../../v8/include \
    gen/webkit \
    ../../third_party/WebKit/Source/Platform/chromium \
    ../../third_party/WebKit/Source/Platform/chromium \
    gen/webcore_headers \
    ../../third_party/npapi \
    ../../third_party/npapi/bindings \
    ../../third_party/WebKit/Tools/DumpRenderTree/chromium/TestRunner/public \
    ../../third_party/WebKit/Source \
    ../../third_party/freetype2/include \
    ../../third_party/freetype2/src/include

for (inc, cr_inc_paths) {
    INCLUDEPATH += $$BUILD_DIR/$$inc
}

CONFIG += link_pkgconfig
PKGCONFIG_PRIVATE = gdk-2.0

DEFINES += QT_NO_KEYWORDS

!chromium_is_static: QMAKE_RPATHDIR += $${BUILD_DIR}/lib

QMAKE_CXXFLAGS += -fno-rtti
