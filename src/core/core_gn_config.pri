CONFIG = gn_generator $$CONFIG
GN_SRC_DIR = $$PWD
GN_FILE = $$OUT_PWD/BUILD.gn
GN_FIND_MOCABLES_SCRIPT = $$shell_path($$QTWEBENGINE_ROOT/tools/scripts/gn_find_mocables.py)
GN_RUN_BINARY_SCRIPT = $$shell_path($$QTWEBENGINE_ROOT/tools/scripts/gn_run_binary.py)

