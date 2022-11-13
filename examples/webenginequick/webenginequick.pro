TEMPLATE=subdirs

SUBDIRS += \
    customdialogs \
    customtouchhandle \
    minimal \
    quicknanobrowser \
    webengineaction

qtHaveModule(quickcontrols2) {
    SUBDIRS += \
        lifecycle \
        recipebrowser
}
