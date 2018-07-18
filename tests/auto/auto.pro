TEMPLATE = subdirs

qtHaveModule(webengine) {
    SUBDIRS += quick
}

qtHaveModule(webenginewidgets) {
    SUBDIRS += core widgets
}

qtHaveModule(pdf) {
    SUBDIRS += pdf
}

