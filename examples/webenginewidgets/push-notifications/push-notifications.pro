QT += webenginewidgets

HEADERS   = notificationpopup.h

SOURCES   = main.cpp

RESOURCES = data/data.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/push-notifications
INSTALLS += target
