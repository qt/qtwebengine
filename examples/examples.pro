TEMPLATE=subdirs

qtHaveModule(webengine): SUBDIRS += webengine

qtHaveModule(webenginewidgets): SUBDIRS += webenginewidgets

qtHaveModule(pdfwidgets): SUBDIRS += pdfwidgets

qtHaveModule(quick): qtHaveModule(pdf): qtHaveModule(pdfwidgets): SUBDIRS += pdf
