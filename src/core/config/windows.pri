GYP_ARGS += "-D qt_os=\"win32\""

# Libvpx build needs additional search path on Windows.
git_chromium_src_dir = $$system("git config qtwebengine.chromiumsrcdir")
GYP_ARGS += "-D qtwe_chromium_obj_dir=\"$$OUT_PWD/$$getConfigDir()/obj/$$git_chromium_src_dir\""

# Use path from environment for perl, bison and gperf instead of values set in WebKit's core.gypi.
GYP_ARGS += "-D perl_exe=\"perl.exe\" -D bison_exe=\"bison.exe\" -D gperf_exe=\"gperf.exe\""

