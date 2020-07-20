
qtConfig(debug_and_release): CONFIG += debug_and_release

TARGET = QtPdf
TEMPLATE = aux

build_pass|!debug_and_relase {
      linking_pri = $$OUT_PWD/$$getConfigDir()/$${TARGET}.pri
      !include($$linking_pri) {
          error("Could not find the linking information that gn should have generated.")
      }

      !isEmpty(NINJA_ARCHIVES) {
          prl_file = $$OUT_PWD/$$getConfigDir()/$${TARGET}_static_dep.pri
          ninja_archives = $$eval($$list($$NINJA_ARCHIVES))
          qqt_libdir = \$\$\$\$[QT_INSTALL_LIBS]
          for(ninja_arch, ninja_archives) {
              ninja_arch_name = $$basename(ninja_arch)
              ninja_arch_dirname = $$dirname(ninja_arch)
              prl_content += "ninja_arch_prl_replace_$${ninja_arch_name}.match = $${ninja_arch_dirname}"
              prl_content += "ninja_arch_prl_replace_$${ninja_arch_name}.replace = $${qqt_libdir}/static_chrome"
              prl_content += "ninja_arch_prl_replace_$${ninja_arch_name}.CONFIG = path"
              prl_content += "QMAKE_PRL_INSTALL_REPLACE += ninja_arch_prl_replace_$${ninja_arch_name}"
          }
          write_file($${prl_file}, prl_content)
      }
}
