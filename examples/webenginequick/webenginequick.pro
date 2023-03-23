TEMPLATE=subdirs

SUBDIRS += \
    quicknanobrowser

qtHaveModule(quickcontrols2) {
    SUBDIRS += \
        lifecycle
}
