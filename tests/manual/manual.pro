TEMPLATE = subdirs

SUBDIRS += \
    widgets

!qtHaveModule(webenginewidgets): SUBDIRS -= widgets
