include(common.pri)

# Reuse the cached sdk version value from mac/sdk.prf if available
# otherwise query for it.
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
GYP_CONFIG += \
    qt_os=\"mac\" \
    mac_sdk_min=\"$${QMAKE_MAC_SDK_VERSION}\" \
    mac_deployment_target=\"$${QMAKE_MACOSX_DEPLOYMENT_TARGET}\" \
    make_clang_dir=\"$${QMAKE_CLANG_DIR}\" \
    clang_use_chrome_plugins=0 \
    enable_widevine=1

QMAKE_MAC_SDK_PATH = "$$eval(QMAKE_MAC_SDK.$${QMAKE_MAC_SDK}.path)"
exists($$QMAKE_MAC_SDK_PATH): GYP_CONFIG += mac_sdk_path=\"$${QMAKE_MAC_SDK_PATH}\"
