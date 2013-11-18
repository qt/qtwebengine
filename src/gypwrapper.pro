# This .pro file serves a dual purpose:
# 1) invoking gyp through the gyp_qtwebengine script, which in turn makes use of the generated gypi include files
# 2) produce a Makefile that will run ninja, and take care of actually building everything.

TEMPLATE = aux

message(Running Gyp...)
!system("python $$QTWEBENGINE_ROOT/tools/buildscripts/gyp_qtwebengine"): error("-- running gyp_qtwebengine failed --")

ninja.target = invoke_ninja
ninja.commands = $$findOrBuildNinja() $$(NINJAFLAGS) -C $$getOutDir()/$$getConfigDir()
ninja.depends: qmake
QMAKE_EXTRA_TARGETS += ninja

build_pass:build_all:default_target.target = all
else: default_target.target = first
default_target.depends = ninja

QMAKE_EXTRA_TARGETS += default_target
