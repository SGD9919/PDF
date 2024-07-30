DEFINES += __QT__ \
    OPJ_STATIC \
    PNG_PREFIX \
    PNG_USE_READ_MACROS

CONFIG += c++1
QMAKE_CXXFLAGS += -std=c++11
    
include($$PWD/../3rdparty/pdfium.pri)

PRIVATE_HEADERS += \
    $$PWD/qpdfiumglobal.h

PUBLIC_HEADERS += \
    $$PWD/qpdfium.h \
    $$PWD/qpdfiumpage.h
SOURCES += \
    $$PWD/qpdfiumglobal.cpp \
    $$PWD/qpdfium.cpp \
    $$PWD/qpdfiumpage.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS


INCLUDEPATH +=$$PWD
DEPENDPATH +=$$PWD

