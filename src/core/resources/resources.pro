# This is a dummy .pro file used to prepare chromium .pak resource files.
# These files will then be bundled using the Qt Resource System.
TEMPLATE = aux

system("python $$QTWEBENGINE_ROOT/tools/buildscripts/build_resources.py")
