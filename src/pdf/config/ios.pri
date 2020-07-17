load(functions)

include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/pdf/qtpdf-config.pri)
QT_FOR_CONFIG += buildtools-private pdf-private

clang_dir = $$which($${QMAKE_CXX})
clang_dir = $$clean_path("$$dirname(clang_dir)/../")

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
toolkit_views=false \
treat_warnings_as_errors=false \
safe_browsing_mode=0 \
optimize_webui=false \
forbid_non_component_debug_builds=false \
clang_use_chrome_plugins=false \
use_xcode_clang=true \
clang_base_path=\"$${clang_dir}\" \
ios_enable_code_signing=false \
target_os=\"ios\" \
ios_deployment_target=\"$${QMAKE_IOS_DEPLOYMENT_TARGET}\" \
enable_ios_bitcode=true \
use_jumbo_build=false

device:simulator {
  # we do fat libray
  gn_args+= \
  target_cpu=\"$${QMAKE_APPLE_DEVICE_ARCHS}\" \
  use_qt_fat_lib=true \
  arm_use_neon=false\
  # note this adds one arch of simulator at the moment, see also additional_target_cpus
  target_sysroot=\"$$xcodeSDKInfo(Path, $$device.sdk)\" \
  additional_target_sysroot=[\"$$xcodeSDKInfo(Path, $$simulator.sdk)\"]
} else {
  simulator {
    equals(QMAKE_APPLE_SIMULATOR_ARCHS,"x86_64") {
      gn_args+=target_cpu=\"x64\"
    } else {
      gn_args+=target_cpu=\"$${QMAKE_APPLE_SIMULATOR_ARCHS}\"
    }
    gn_args+=target_sysroot=\"$$xcodeSDKInfo(Path, $$simulator.sdk)\"
  }
  device {
    gn_args+=target_cpu=\"$${QMAKE_APPLE_DEVICE_ARCHS}\"
    gn_args+=target_sysroot=\"$$xcodeSDKInfo(Path, $$device.sdk)\"
  }
}
