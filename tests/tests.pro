TEMPLATE = subdirs

SUBDIRS +=  auto

qtHaveModule(webengine) {
    SUBDIRS += quicktestbrowser
}
