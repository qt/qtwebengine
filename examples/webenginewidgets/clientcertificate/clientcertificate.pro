QT_FOR_CONFIG += network-private
TEMPLATE = subdirs

client.file = client.pro
server.file = server.pro

qtConfig(ssl): SUBDIRS += client server
