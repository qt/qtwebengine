TEMPLATE = subdirs

SUBDIRS += \
    inspectorserver \
    publicapi \
    qquickwebenginedefaultsurfaceformat \
    qquickwebengineview \
    qquickwebengineviewgraphics

isQMLTestSupportApiEnabled(): SUBDIRS += qmltests
