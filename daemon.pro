QT += network
QT -= gui
TARGET = input-event-daemon
*-g++*:QMAKE_CXXFLAGS += -std=c++0x

SOURCES += \
    main.cpp \
    keystreamserver.cpp \
    inputdevice.cpp

HEADERS += \
    keystreamserver.h \
    inputdevice.h

target.path = /usr/bin
INSTALLS += target

systemd-service.path = /lib/systemd/system/
systemd-service.files = input-event-daemon.service
INSTALLS += systemd-service
