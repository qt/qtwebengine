load(functions)

include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/pdf/qtpdf-config.pri)
QT_FOR_CONFIG += buildtools-private pdf-private


QMAKE_MAC_SDK_VERSION = $$eval(QMAKE_MAC_SDK.$${QMAKE_MAC_SDK}.SDKVersion)
isEmpty(QMAKE_MAC_SDK_VERSION) {
    QMAKE_MAC_SDK_VERSION = $$system("/usr/bin/xcodebuild -sdk $${QMAKE_MAC_SDK} -version SDKVersion 2>/dev/null")
    isEmpty(QMAKE_MAC_SDK_VERSION): error("Could not resolve SDK version for \'$${QMAKE_MAC_SDK}\'")
}
QMAKE_CLANG_DIR = "/usr"
QMAKE_CLANG_PATH = $$eval(QMAKE_MAC_SDK.macx-clang.$${QMAKE_MAC_SDK}.QMAKE_CXX)
!isEmpty(QMAKE_CLANG_PATH) {
    clang_dir = $$clean_path("$$dirname(QMAKE_CLANG_PATH)/../")
    exists($$clang_dir): QMAKE_CLANG_DIR = $$clang_dir
}
QMAKE_CLANG_PATH = "$${QMAKE_CLANG_DIR}/bin/clang++"
message("Using clang++ from $${QMAKE_CLANG_PATH}")
system("$${QMAKE_CLANG_PATH} --version")

gn_args += \
use_qt=true \
closure_compile=false \
is_component_build=false \
is_shared=true \
is_debug=true \
enable_message_center=false \
enable_nacl=false \
enable_remoting=false \
enable_reporting=false \
enable_resource_whitelist_generation=false \
enable_swiftshader=false \
enable_web_speech=false \
has_native_accessibility=false \
enable_debugallocation=false \
use_allocator_shim=false \
use_allocator=\"none\" \
use_custom_libcxx=false \
v8_use_external_startup_data=false \
v8_use_snapshot=false \
toolkit_views=false \
treat_warnings_as_errors=false \
safe_browsing_mode=0 \
optimize_webui=false \
forbid_non_component_debug_builds=false \
clang_use_chrome_plugins=false \
use_xcode_clang=true \
clang_base_path=\"$${QMAKE_CLANG_DIR}\" \
ios_enable_code_signing=false \
target_os=\"ios\" \
#target_cpu=\"$${QMAKE_APPLE_SIMULATOR_ARCHS}\" \
#target_cpu=\"$${QMAKE_APPLE_DEVICE_ARCHS}\" \
target_cpu=\"x64\" \
ios_deployment_target=\"$${QMAKE_IOS_DEPLOYMENT_TARGET}\" \
enable_ios_bitcode=true \
use_jumbo_build=false \
pdf_enable_v8=false \
pdf_enable_xfa=false \
pdf_enable_xfa_bmp=false \
pdf_enable_xfa_gif=false \
pdf_enable_xfa_png=false \
pdf_enable_xfa_tiff=false
