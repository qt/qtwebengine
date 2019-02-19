TEMPLATE = subdirs

SUBDIRS += \
    qwebengineclientcertificatestore \
    qwebenginecookiestore \
    qwebengineurlrequestinterceptor \

# QTBUG-60268
boot2qt: SUBDIRS = ""
