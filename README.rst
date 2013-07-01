This is a prototype of allowing embedding Chromium/Blink into Qt.

-- Build instructions:

(1) Clone the QtWebEngine repository

    * git clone git://gitorious.org/qt-labs/qtwebengine.git

(2) Initialize the repository

    This will clone the chromium sources and apply some necessary patches on top.

    * ./init-repository.py

(3) Generate the ninja build files by running qmake.

    Use qmake -r to forcefully re-gyp (without relying on make to determine if it's necessary).

    * qmake

(4) build with make ;)

    Release or debug builds can be obtained by running 'make release' or 'make debug' in the
    top level directory (only lib and process for now, and not so smart with dependencies)

(5) Additional tweaks and tips:
    * linking all the static librairies can be a slow and painful process when developing (especially with debug builds).
      Use of shared librairies can be enforced by setting the GYP_DEFINES environment variable, like so:

      export GYP_DEFINES=component=shared_library

      Or by using the include.gypi mechanism described below:

    * On Linux you may get build errors due to -Werror. Create ~/.gyp/include.gypi with the following contents:
      {
          'variables': {
              'werror%': '',
              'component%': 'shared_library',
          },
      }

      So afterwards you have to re-create the ninja files by running "qmake -r"

    * If you want to use external chromium sources instead of the submodule provided in the QtWebEngine repository,
      you can export the CHROMIUM_SRC_DIR variable pointint to your source directory.


