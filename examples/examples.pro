TEMPLATE=subdirs

SUBDIRS += webengine/quicknanobrowser

qtHaveModule(webenginewidgets) {
    SUBDIRS += \
        webenginewidgets/browser \
        webenginewidgets/fancybrowser
}
