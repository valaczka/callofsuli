DEFINES += WITH_CGRAPH

unix {
 CONFIG += link_pkgconfig
 PKGCONFIG += libgvc #libcdt libgvc libcgraph #libgraph
}

win32 {
 GRAPHVIZ_PATH = "D:/Program Files (x86)/Graphviz2.36/"
 DEFINES += WIN32_DLL
 DEFINES += GVDLL
 INCLUDEPATH += $$GRAPHVIZ_PATH/include/graphviz
 LIBS += -L$$GRAPHVIZ_PATH/lib/release/lib -lgvc #-lcgraph -lgraph -lcdt
}

macx {
 # Enable pkg-config (pkg-config is disabled by default in the Qt package for mac)
 QT_CONFIG -= no-pkg-config
 # pkg-config location if your brew installation is standard
 PKG_CONFIG = /usr/local/bin/pkg-config
 # libgraph is no longer present in last graphviz version
 PKGCONFIG -= libgraph
}
