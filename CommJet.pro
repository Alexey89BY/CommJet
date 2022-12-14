QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    commpropsdialog.cpp \
    consolewindow.cpp \
    main.cpp \
    mainwindow.cpp \
    HexParser.cpp \
    SAX.cpp \
    NTCB.cpp \
    crc.cpp \
    cheksum.cpp \
    miniconfigdialog.cpp

HEADERS += \
    commpropsdialog.h \
    consolewindow.h \
    mainwindow.h \
    HexParser.hpp \
    SAX.hpp \
    NTCB.hpp \
    crc.hpp \
    cheksum.hpp \
    miniconfigdialog.h

FORMS += \
    commpropsdialog.ui \
    consolewindow.ui \
    mainwindow.ui \
    miniconfigdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

RC_ICONS = resource/servers.ico
