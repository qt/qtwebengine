GYP_ARGS += "-D qt_os=\"win32\" -I config/windows.gypi"

GYP_CONFIG += \
    disable_nacl=1 \
    remoting=0 \
    use_ash=0 \

# Libvpx build needs additional search path on Windows.
git_chromium_src_dir = $$system("git config qtwebengine.chromiumsrcdir")
GYP_ARGS += "-D qtwe_chromium_obj_dir=\"$$OUT_PWD/$$getConfigDir()/obj/$$git_chromium_src_dir\""

# Use path from environment for perl, bison and gperf instead of values set in WebKit's core.gypi.
GYP_ARGS += "-D perl_exe=\"perl.exe\" -D bison_exe=\"bison.exe\" -D gperf_exe=\"gperf.exe\""

# Gyp's parallel processing is broken on Windows
GYP_ARGS += "--no-parallel"

