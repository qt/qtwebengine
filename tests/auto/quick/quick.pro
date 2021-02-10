include($$QTWEBENGINE_OUT_ROOT/src/webenginequick/qtwebenginequick-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webenginequick-private

TEMPLATE = subdirs

SUBDIRS += \
    dialogs \
    inspectorserver \
    qmltests \
    publicapi \
    qquickwebenginedefaultsurfaceformat \
    qquickwebengineview \
    qtbug-70248 \
    certificateerror

qtConfig(webenginequick-testsupport) {
    SUBDIRS += qquickwebengineviewgraphics
}

boot2qt: SUBDIRS -= inspectorserver qquickwebengineview qmltests
