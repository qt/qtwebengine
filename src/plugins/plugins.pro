TEMPLATE = subdirs
qtHaveModule(webenginewidgets): qtHaveModule(designer): SUBDIRS += qwebengineview
qtHaveModule(pdf): qtConfig(imageformatplugin): SUBDIRS += imageformats
