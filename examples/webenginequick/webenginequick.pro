TEMPLATE=subdirs

SUBDIRS += \
    customtouchhandle \
    quicknanobrowser \
    webengineaction

qtHaveModule(quickcontrols2) {
    SUBDIRS += \
        lifecycle \
        recipebrowser
}
