TEMPLATE = subdirs

SUBDIRS +=  auto

qtHaveModule(webengine-quick) {
    SUBDIRS += quicktestbrowser
}
