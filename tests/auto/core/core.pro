TEMPLATE = subdirs
QT_FOR_CONFIG += network-private

SUBDIRS += \
    qwebenginecookiestore \
    qwebengineurlrequestinterceptor \

qtConfig(ssl): SUBDIRS += qwebengineclientcertificatestore

# QTBUG-60268
boot2qt: SUBDIRS = ""
