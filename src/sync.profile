%modules = ( # path to module name map
    "QtWebEngineQuick" => "$basedir/webenginequick",
    "QtWebEngineWidgets" => "$basedir/webenginewidgets",
    "QtWebEngineTestSupport" => "$basedir/webenginequick/testsupport",
    "QtWebEngineCore" => "$basedir/core/api",
    "QtPdf" => "$basedir/pdf",
    "QtPdfWidgets" => "$basedir/pdfwidgets",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
    "QtWebEngineQuick" => "api",
    "QtWebEngineWidgets" => "api",
    "QtPdf" => "api"
);
%classnames = (
);
