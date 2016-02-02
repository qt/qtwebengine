TEMPLATE=subdirs

qtHaveModule(webengine) {
    SUBDIRS += webengine/quicknanobrowser
}

qtHaveModule(webenginewidgets) {
    SUBDIRS += \
        webenginewidgets/contentmanipulation \
        webenginewidgets/cookiebrowser \
        webenginewidgets/demobrowser \
        webenginewidgets/markdowneditor
}
