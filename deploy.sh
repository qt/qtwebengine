#!/bin/bash
ANDROID_TOOLS_PATH=`qmake -query QT_HOST_PREFIX`/../../../Tools/b2qt
QTWEBENGINE_ROOT="$PWD"

TARGET="Release"
TARGET_DEPLOY="$QTWEBENGINE_ROOT/out/$TARGET/deploy"

set -x
set -e

if [ "$1" = "-d" ]
then
  rm -rf $TARGET_DEPLOY/*
  mkdir -p $TARGET_DEPLOY/system

  echo "deploying needed libraries and binaries"
  mkdir -p $TARGET_DEPLOY/system/lib
  mkdir -p $TARGET_DEPLOY/system/qml/QtWebEngine/UIDelegates
  mkdir -p $TARGET_DEPLOY/system/qml/QtWebEngine/experimental
  mkdir -p $TARGET_DEPLOY/system/libexec

  cp $QTWEBENGINE_ROOT/libexec/QtWebEngineProcess $TARGET_DEPLOY/system/libexec/QtWebEngineProcess
  cp $QTWEBENGINE_ROOT/lib/libQt5WebEngineWidgets.so $TARGET_DEPLOY/system/lib/libQt5WebEngineWidgets.so.5
  cp $QTWEBENGINE_ROOT/lib/libQt5WebEngine.so $TARGET_DEPLOY/system/lib/libQt5WebEngine.so.5
  cp $QTWEBENGINE_ROOT/lib/libQt5WebEngineCore.so.5 $TARGET_DEPLOY/system/lib/libQt5WebEngineCore.so.5

  cp -r $QTWEBENGINE_ROOT/qml/QtWebEngine $TARGET_DEPLOY/system/qml
  cp -r $QTWEBENGINE_ROOT/src/webengine/ui $TARGET_DEPLOY/system/qml/QtWebEngine/UIDelegates
  cp -r $QTWEBENGINE_ROOT/src/webengine/plugin/qmldir $TARGET_DEPLOY/system/qml/QtWebEngine/qmldir
  cp -r $QTWEBENGINE_ROOT/src/webengine/plugin/experimental/qmldir $TARGET_DEPLOY/system/qml/QtWebEngine/experimental/qmldir
  cp -r $QTWEBENGINE_ROOT/src/core/Release/gen/repack/qtwebengine_resources.pak $TARGET_DEPLOY/system/qtwebengine_resources.pak

  ${CROSS_COMPILE}strip --strip-unneeded $TARGET_DEPLOY/system/lib/libQt5WebEngineCore.so.5
  ${CROSS_COMPILE}strip --strip-unneeded $TARGET_DEPLOY/system/lib/libQt5WebEngineWidgets.so.5
  ${CROSS_COMPILE}strip --strip-unneeded $TARGET_DEPLOY/system/lib/libQt5WebEngine.so.5
  ${CROSS_COMPILE}strip --strip-unneeded $TARGET_DEPLOY/system/libexec/QtWebEngineProcess

  cd $TARGET_DEPLOY
  ANDROID_PRODUCT_OUT="$TARGET_DEPLOY" $ANDROID_TOOLS_PATH/adb sync system

  $ANDROID_TOOLS_PATH/adb push $QTWEBENGINE_ROOT/examples/webengine/quicknanobrowser/quicknanobrowser /quicknanobrowser
  $ANDROID_TOOLS_PATH/adb push $QTWEBENGINE_ROOT/examples/webenginewidgets/fancybrowser/fancybrowser /fancybrowser
  $ANDROID_TOOLS_PATH/adb shell "if [ ! -f /usr/local/Qt-5.2.1/libexec/QtWebEngineProcess ] ; then ln -s /system/libexec/QtWebEngineProcess /usr/local/Qt-5.2.1/libexec/QtWebEngineProcess ; fi"
  $ANDROID_TOOLS_PATH/adb shell "if [ ! -f /usr/local/Qt-5.2.1/qtwebengine_resources.pak ] ; then ln -s /system/qtwebengine_resources.pak /usr/local/Qt-5.2.1/qtwebengine_resources.pak ; fi"
  $ANDROID_TOOLS_PATH/adb shell "if [ ! -f /usr/local/Qt-5.2.1/qml/QtWebEngine ] ; then ln -s /system/qml/QtWebEngine /usr/local/Qt-5.2.1/qml/QtWebEngine ; fi"

  for lib in `$ANDROID_TOOLS_PATH/adb shell ls /system/lib/* | tr -d '\r'` ; do
    echo "Creating link in /usr/lib/ for $lib"
    BNAME=`basename $lib`
    $ANDROID_TOOLS_PATH/adb shell "if [ ! -f /usr/lib/$BNAME ] ; then ln -s $lib /usr/lib/$BNAME ; fi"
  done

#$ANDROID_TOOLS_PATH/adb push $QTWEBENGINE_ROOT/target.env /target.env

fi

#$ANDROID_TOOLS_PATH/adb shell "appcontroller gdbserver :5039 /quicknanobrowser"
$ANDROID_TOOLS_PATH/adb shell 
