
#########################
### WASM LIMITATIONS
#########################

# Url not avaliable (CORS)
# Proxy roles not available (too much recursion)

#########################

SOURCES += \
	onlineapplication.cpp \
	onlineclient.cpp

HEADERS += \
	onlineapplication.h \
	onlineclient.h

DESTDIR = ../html

LIBS += -s EXIT_RUNTIME=1

