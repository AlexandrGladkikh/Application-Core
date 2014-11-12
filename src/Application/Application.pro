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


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../appcontroller/Appcontroller-build-desktop-Qt_4_8_2_in_PATH__System__Release/release/ -lAppcontroller
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../appcontroller/Appcontroller-build-desktop-Qt_4_8_2_in_PATH__System__Release/debug/ -lAppcontroller
else:symbian: LIBS += -lAppcontroller
else:unix: LIBS += -L$$PWD/../../appcontroller/Appcontroller-build-desktop-Qt_4_8_2_in_PATH__System__Release/ -lAppcontroller

INCLUDEPATH += $$PWD/../../appcontroller/Appcontroller-build-desktop-Qt_4_8_2_in_PATH__System__Release
DEPENDPATH += $$PWD/../../appcontroller/Appcontroller-build-desktop-Qt_4_8_2_in_PATH__System__Release

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../appcontroller/Appcontroller-build-desktop-Qt_4_8_2_in_PATH__System__Release/release/Appcontroller.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../appcontroller/Appcontroller-build-desktop-Qt_4_8_2_in_PATH__System__Release/debug/Appcontroller.lib
else:unix:!symbian: PRE_TARGETDEPS += $$PWD/../../appcontroller/Appcontroller-build-desktop-Qt_4_8_2_in_PATH__System__Release/libAppcontroller.a
