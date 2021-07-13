%modules = ( # path to module name map
    "QtWebEngineQuick" => "$basedir/src/webenginequick",
    "QtWebEngineWidgets" => "$basedir/src/webenginewidgets",
    "QtWebEngineCore" => "$basedir/src/core/api",
    "QtPdf" => "$basedir/src/pdf",
    "QtPdfWidgets" => "$basedir/src/pdfwidgets",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
    "QtWebEngineQuick" => "api",
    "QtWebEngineWidgets" => "api",
    "QtPdf" => "api"
);
%classnames = (
);
