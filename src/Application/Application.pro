TEMPLATE = app
CONFIG += console
CONFIG -= qt
LIBS  = -lpthread
QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    ../including/AppStatus.cpp \
    ../including/AppMessage.cpp \
    ../including/AppSettings.cpp \
    ../including/AppData.cpp \
    ../including/AppStart.cpp \
    ../Modules/Controller.cpp \
    ../Modules/ModuleData.cpp \
    ../Modules/AppController.cpp \
    ../Modules/NetModule.cpp \
    ../Modules/UI.cpp \
    ../Wrap/WrapNet.cpp \
    ../Wrap/Log.cpp \
    ../Wrap/UserData.cpp

HEADERS += \
    ../including/AppStatus.h \
    ../including/AppMessage.h \
    ../including/AppSettings.h \
    ../including/AppData.h \
    ../including/AppStart.h \
    ../Modules/Controller.h \
    ../Modules/ModuleData.h \
    ../Modules/AppController.h \
    ../Modules/NetModule.h \
    ../Modules/UI.h \
    ../Wrap/WrapNet.h \
    ../Wrap/Log.h \
    ../Wrap/UserData.h \
    ../including/AppDefined.h \
    ../Modules/ModulesDefined.h




unix:!macx: LIBS += -L$$PWD/../../appcontroller/AppController/ -lAppcontroller

INCLUDEPATH += $$PWD/../../appcontroller/AppController
DEPENDPATH += $$PWD/../../appcontroller/AppController

unix:!macx: PRE_TARGETDEPS += $$PWD/../../appcontroller/AppController/libAppcontroller.a
