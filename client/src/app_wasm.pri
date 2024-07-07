
#########################
### WASM LIMITATIONS
#########################

# Url not avaliable (CORS)
# Proxy roles not available (too much recursion)

#########################

QMAKE_CXXFLAGS += -oz -flto
QMAKE_LFLAGS += -flto

SOURCES += \
	onlineapplication.cpp \
	onlineclient.cpp

HEADERS += \
	onlineapplication.h \
	onlineclient.h

DESTDIR = ../html

LIBS += -s EXIT_RUNTIME=1

