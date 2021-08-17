TEMPLATE=subdirs

SUBDIRS += \
    customdialogs \
    minimal \
    quicknanobrowser \
    webengineaction

qtHaveModule(quickcontrols2) {
    SUBDIRS += \
        lifecycle \
        recipebrowser
}
