TEMPLATE=subdirs

SUBDIRS += \
    quicknanobrowser \
    webengineaction

qtHaveModule(quickcontrols2) {
    SUBDIRS += \
        lifecycle \
        recipebrowser
}
