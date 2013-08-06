TEMPLATE=subdirs

SUBDIRS += quick/quicknanobrowser
qtHaveModule(widgets) {
    SUBDIRS += \
        widgets/browser \
        widgets/widgetsnanobrowser
}
