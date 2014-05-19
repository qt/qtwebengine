TEMPLATE=subdirs

SUBDIRS += webengine/quicknanobrowser

qtHaveModule(webenginewidgets):equals(QT_MAJOR_VERSION, 5):greaterThan(QT_MINOR_VERSION, 2) {
    SUBDIRS += \
        webenginewidgets/browser \
        webenginewidgets/fancybrowser
}
