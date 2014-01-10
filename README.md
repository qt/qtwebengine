# QtWebEngine - Combining the power of Chromium and Qt #


To be able to build QtWebEngine you need Qt5 or newer.

## Build Instructions##
### (1) Clone the QtWebEngine repository ###

    git clone git://gitorious.org/qt-labs/qtwebengine.git

### (2) Initialize the repository ###

This will clone a submodule called 3rdparty that contains a snapshot of chromium sources.
In case that any patches are necessary on top of the regular chromium sources,
they will already be included in this snapshot.

    git submodule init && git submodule update --recursive

### (3) Generate the ninja build files by running qmake. ###

Use qmake -r to forcefully re-gyp (without relying on make to determine if it's necessary).

    qmake

### (4) build with make ;) ###

Release or debug builds can be obtained by running 'make release' or 'make debug' in the
top level directory (only lib and process for now, and not so smart with dependencies)

    make

## Additional tips and tricks ##

### Complete Upstream Chromium Checkout ###
If you want to have a complete chromium checkout with the complete history instead of the snapshot,
then do not run step (2). Instead just run the init-repository.py script.

This will then create a complete ninja and chromium checkout in the subdirectory 3rdparty_upstream.
qmake will automatically pickup the location and make use of the sources in the subsequent step (3) and (4).


    ./init-repository.py

### Use shared libraries instead of static ones ###
Linking all the static libraries can be a slow and painful process when developing (especially with debug builds).
Use of shared libraries can be enforced by setting the GYP_DEFINES environment variable, like so:


    export GYP_DEFINES=component=shared_library

  Or by using the ~/.gyp/include.gypi mechanism.

### Use external Chromium sources ###
If you want to use external chromium sources instead of the submodule provided in the QtWebEngine repository,
you can export the CHROMIUM_SRC_DIR variable pointint to your source directory.


