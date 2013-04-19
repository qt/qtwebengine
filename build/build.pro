# This .pro file serves a dual purpose:
# 1) invoking gyp through the gyp_blinq script, which in turn makes use of the generated gypi include files
# 2) produce a Makefile that will run ninja, and take care of actually building everything.

TEMPLATE = aux

# Fetched from environment for now
CHROMIUM_SRC_DIR = $$(CHROMIUM_SRC_DIR)
isEmpty(CHROMIUM_SRC_DIR):error("please set CHOMIUM_SRC_DIR")

message(Running Gyp...)
GYP_OUTPUT = $$system(./gyp_blinq)
message($$GYP_OUTPUT)

ninja.target = ninja
# FIXME: Don't hardcode Release... might be tricky to get right if we also don't want to hardcode 'out'
ninja.commands = $$CHROMIUM_SRC_DIR/../depot_tools/ninja -C $$BLINQ_ROOT/out/Release
ninja.depends: qmake
QMAKE_EXTRA_TARGETS += ninja

build_pass:build_all:default_target.target = all
else: default_target.target = first
default_target.depends = ninja

QMAKE_EXTRA_TARGETS += default_target
