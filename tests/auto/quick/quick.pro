TEMPLATE = subdirs

SUBDIRS += \
    qquickwebviewgraphics/qquickwebviewgraphics_software.pro \
    qquickwebengineview/qquickwebengineview.pro \

equals(QT_MAJOR_VERSION, 5):greaterThan(QT_MINOR_VERSION, 1): SUBDIRS += qquickwebviewgraphics/qquickwebviewgraphics.pro
