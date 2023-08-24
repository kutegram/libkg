QT += network xml sql

include($$PWD/qt-json/qt-json.pri)
include($$PWD/zlib/zlib.pri)
include($$PWD/mbedtls/mbedtls.pri)

HEADERS += \
    $$PWD/apisecrets.h \
    $$PWD/tlschema.h \
    $$PWD/mtschema.h \
    $$PWD/tgstream.h \
    $$PWD/crypto.h \
    $$PWD/tgclient.h \
    $$PWD/tgtransport.h \
    $$PWD/systemname.h \
    $$PWD/qcompressor.h \
    $$PWD/debug.h \
    $$PWD/tgutils.h

SOURCES += \
    $$PWD/tlschema.cpp \
    $$PWD/mtschema.cpp \
    $$PWD/tgstream.cpp \
    $$PWD/crypto.cpp \
    $$PWD/tgclient.cpp \
    $$PWD/tgtransport.cpp \
    $$PWD/systemname.cpp \
    $$PWD/qcompressor.cpp \
    $$PWD/tgc_auth.cpp \
    $$PWD/tgc_help.cpp \
    $$PWD/tgc_messages.cpp \
    $$PWD/tgutils.cpp \
    $$PWD/tgc_files.cpp \
    $$PWD/division.c

INCLUDEPATH += $$PWD
