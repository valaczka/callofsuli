DESTDIR = ..

include(modules.pri)

equals(MODULE_BUILD, static): CONFIG += static
