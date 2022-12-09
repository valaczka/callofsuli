include(common.pri)

TEMPLATE = subdirs

SUBDIRS += \
				version \
				client/lib \
				application
#                client/Bacon2D-static \
#                client/QtXlsxWriter \
#                client/QZXing \
#                client/app/modules \
#                client/app

application.file = client/src/app.pro

CONFIG += ordered
