QT += network xml sql

include(json/qt-json.pri)
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
    $$PWD/telegramstream.h \
    $$PWD/crypto.h

SOURCES += \
    $$PWD/tlschema.cpp \
    $$PWD/mtschema.cpp \
    $$PWD/telegramstream.cpp \
    $$PWD/crypto.cpp

INCLUDEPATH += $$PWD
