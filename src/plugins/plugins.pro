TEMPLATE = subdirs
qtHaveModule(webengine-widgets): qtHaveModule(designer): SUBDIRS += qwebengineview
qtHaveModule(pdf): qtConfig(imageformatplugin): SUBDIRS += imageformats
