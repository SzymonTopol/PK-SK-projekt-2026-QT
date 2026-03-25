QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ARX.cpp \
    FunctionGenerator.cpp \
    RegulatorPID.cpp \
    ServicesManager.cpp \
    UAR.cpp \
    arx_change_limits.cpp \
    arx_change_popup.cpp \
    main.cpp \
    mainwindow.cpp \
    saveloadmanager.cpp

HEADERS += \
    ARX.h \
    FunctionGenerator.h \
    RegulatorPID.h \
    ServicesManager.h \
    UAR.h \
    arx_change_limits.h \
    arx_change_popup.h \
    mainwindow.h \
    saveloadmanager.h

FORMS += \
    arx_change_limits.ui \
    arx_change_popup.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
