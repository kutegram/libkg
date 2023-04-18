QT += network xml sql

include(qt-json/qt-json.pri)
include(thirdparty/thirdparty.pri)

symbian:LIBS += -llibcrypto

!symbian:!android:unix:LIBS += -lz
!symbian:!android:unix:LIBS += -lcrypto

android:LIBS += -lz
android:LIBS += -LC:/QtAndroid/openssl -lcrypto
android:INCLUDEPATH += C:/QtAndroid/openssl/include

win32:include(zlib/zlib.pri)
win32:LIBS += -LC:/OpenSSL-Win32/lib -llibcrypto
win32:INCLUDEPATH += C:/OpenSSL-Win32/include

HEADERS += \
    $$PWD/apisecrets.h \
    $$PWD/tlschema.h \
    $$PWD/mtschema.h \
    $$PWD/tgstream.h \
    $$PWD/crypto.h \
    $$PWD/tgclient.h \
    $$PWD/tgtransport.h \
    $$PWD/systemname.h \
    $$PWD/qcompressor.h

SOURCES += \
    $$PWD/tlschema.cpp \
    $$PWD/mtschema.cpp \
    $$PWD/tgstream.cpp \
    $$PWD/crypto.cpp \
    $$PWD/tgclient.cpp \
    $$PWD/tgtransport.cpp \
    $$PWD/systemname.cpp \
    $$PWD/qcompressor.cpp \
    $$PWD/division.c

INCLUDEPATH += $$PWD
