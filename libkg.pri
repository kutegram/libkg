QT += network xml sql

include(qt-json/qt-json.pri)

symbian:LIBS += -llibcrypto

!symbian:!android:unix:LIBS += -lz
!symbian:!android:unix:LIBS += -lcrypto

android:LIBS += -lz
android:INCLUDEPATH += C:/Users/Mathew/Desktop/Android/openssl-1.0.2j/include
android:LIBS += C:/Users/Mathew/Desktop/Android/openssl-1.0.2j/libcrypto.a
android:ANDROID_EXTRA_LIBS = C:/Users/Mathew/Desktop/Android/openssl-1.0.2j/libcrypto.so

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
    $$PWD/tgc_files.cpp

INCLUDEPATH += $$PWD
