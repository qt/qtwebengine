TEMPLATE=subdirs

SUBDIRS += \
    customdialogs \
    customtouchhandle \
    quicknanobrowser \
    webengineaction

qtHaveModule(quickcontrols2) {
    SUBDIRS += \
        lifecycle \
        recipebrowser
}
