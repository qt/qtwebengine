include($$QTWEBENGINE_OUT_ROOT/src/webengine/qtwebengine-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webengine-private

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

qtConfig(webengine-testsupport) {
    SUBDIRS += qquickwebengineviewgraphics
}

boot2qt: SUBDIRS -= inspectorserver qquickwebengineview qmltests
