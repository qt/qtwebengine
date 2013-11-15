%modules = ( # path to module name map
    "QtWebEngine" => "$basedir/lib/quick",
    "QtWebEngineWidgets" => "$basedir/lib/widgets",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
    "QtWebEngineWidgets" => "Api",
);
%classnames = (
);

# Module dependencies.
# Every module that is required to build this module should have one entry.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - any git symbolic ref resolvable from the module's repository (e.g. "refs/heads/master" to track master branch)
#
%dependencies = (
    "qtbase" => "refs/heads/stable",
    "qtdeclarative" => "refs/heads/stable",
    "qtxmlpatterns" => "refs/heads/stable",
# FIXME: take examples out into their own module to avoid a potential circular dependency later ?
    "qtquickcontrols" => "refs/heads/stable",
);
