# Build Call of Suli server
CONFIG += server

#----

TEMPLATE = subdirs

CONFIG += ordered


unix: SUBDIRS += src/version

server: SUBDIRS += src/server

client: SUBDIRS += src/client

