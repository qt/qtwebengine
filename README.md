# QtWebEngine - Combining the power of Chromium and Qt #


To be able to build QtWebEngine you need Qt 5.2 or newer.

## I. Getting the Code ##

### 1) Clone the QtWebEngine repository ###

    git clone git://gitorious.org/qt-labs/qtwebengine.git

### 2) Initialize the repository ###

This will fetch a snapshot of chromium sources we rely on.

    ./init-repository.py

## II. Build Instructions##

### 1) Generate the ninja build files by running qmake. ###

It's a also possible to use qmake -r to forcefully re-gyp (without relying on make to determine if it's necessary).

    qmake

### 2) build with make ###

Everything should be set up properly now.

    make

### 3) [optional] make install ###

This step is required for installing l10n files and other resources (such as the resources for the remote inspector).

    make install

## Additional tips and tricks ##

### Complete Upstream Chromium Checkout ###
If you want to have a complete chromium checkout with the complete history instead of the snapshot,
then you can run the init-repository script with the -u option.

This will then create a complete ninja and chromium checkout in the subdirectory src/3rdparty\_upstream.
qmake will automatically pickup the location and make use of the sources in the subsequent steps II.1) and II.2).

    ./init-repository.py -u

### Use external Chromium sources ###
If you want to use external chromium sources instead of the submodule provided in the QtWebEngine repository,
you can export the CHROMIUM\_SRC\_DIR variable point it to your source directory.

### Debug vs. Release builds ###

By default, the configuration used for building Qt is followed.
It is possible to override this by passing CONFIG+=release or debug at qmake time. e.g:

    qmake -r CONFIG+=debug

