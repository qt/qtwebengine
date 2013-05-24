This is a prototype of allowing embedding Chromium/Blink into Qt.

-- Build instructions:

(1) Get the Chromium source code, see instructions at http://www.chromium.org/blink/conversion-cheatsheet
    Currently this is a bit iffy due to the WebKit -> Blink transition, but this is what worked for me:

    * git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    * Add depot_tools to your PATH
    * ubuntu>$ sudo apt-get build-dep chromium-browser
    * fetch blink --nosvn=True
    * If that didn't do the trick, it should essentially boil down to running "gclient sync"

(2) Apply the necessary patches (located in patches/ subdir) to chromium's source tree.
    We have a half-baked script that automates updating and patching, with a few limitations:
        1. it has to be run from the patches subdir
        2. the affected git repos have to be in a clean state regarding am and rebase (i.e. no rebase-apply directory)

(3) (Re-)generating the ninja build files after changing a gyp file:

    * set the CHROMIUM_SRC_DIR environment variable to point to /path/to/src/
    * Simply run qmake in the top-level directory (it will call ninja behind the scenes).
    Use qmake -r to forcefully re-gyp (without relying on make to determine if it's necessary).
    * Release or debug builds can be obtained by running 'make release' or 'make debug' in the
      top level directory (only lib and process for now, and not so smart with dependencies)

(4) build with make ;)

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

