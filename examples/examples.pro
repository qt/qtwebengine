TEMPLATE=subdirs

SUBDIRS += webengine/quicknanobrowser
qtHaveModule(widgets) {
    SUBDIRS += \
        webenginewidgets/browser \
        webenginewidgets/fancybrowser
}
