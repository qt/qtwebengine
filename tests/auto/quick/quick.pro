TEMPLATE = subdirs

SUBDIRS += \
    inspectorserver \
    publicapi \
    qquickwebengineview \
    qquickwebengineviewgraphics

isQMLTestSupportApiEnabled(): SUBDIRS += qmltests
