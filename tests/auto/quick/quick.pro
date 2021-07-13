include($$QTWEBENGINE_OUT_ROOT/src/webenginequick/qtwebenginequick-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webenginequick-private

TEMPLATE = subdirs

SUBDIRS += \
    dialogs \
    inspectorserver \
    qmltests \
    publicapi \
    qquickwebenginedefaultsurfaceformat \
    qquickwebengineviewgraphics \
    qquickwebengineview \
    qtbug-70248

boot2qt: SUBDIRS -= inspectorserver qquickwebengineview qmltests
