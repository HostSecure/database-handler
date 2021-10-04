QT -= gui
QT += sql mqtt

CONFIG += c++2a
CONFIG -= app_bundle

# If release build
CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

INCLUDEPATH += src

DEPENDPATH += ../messagehandler/lib/include
INCLUDEPATH += ../messagehandler/lib/include
LIBS += -L../messagehandler/lib -lmessagehandler

SOURCES += \
    main.cpp \
    src/databasedatafileparser.cpp \
    src/databasehandler.cpp \
    src/databasemanager.cpp \
    src/databasemqttclient.cpp \
    src/loghandler.cpp \
    src/testhandler.cpp

HEADERS += \
    src/databasedatafileparser.h \
    src/databasehandler.h \
    src/databasemanager.h \
    src/databasemqttclient.h \
    src/loghandler.h \
    src/testhandler.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


