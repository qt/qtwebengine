TEMPLATE = subdirs
QT_FOR_CONFIG += network-private

SUBDIRS += \
    qwebenginecookiestore \
    qwebenginesettings \
    qwebengineurlrequestinterceptor \
    devtools \
    origins

qtConfig(ssl): SUBDIRS += certificateerror qwebengineclientcertificatestore

