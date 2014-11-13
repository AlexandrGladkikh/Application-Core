#ifndef APPDATA_H
#define APPDATA_H

#pragma once

namespace app {
////////////////////////////////////

enum ModuleName
{
    ModuleNameFirst = 0,
    UI = 0,
    NewAppModule = 1,
    NewNetModule = 2,
    controller = 3,
    //moduleData = 4,
    appController = 4,
    netModule = 5,
    ModuleNameEnd = 6,
    UnavailableName = -1
};

enum ModuleStatus             // здесь указаны состояния модулей
{
    StatusUnavailable,        // устанавливаетс в зависимости от того чем занят в текущий момент времени модуль
    StatusWait,               // необходимо для уведомления других модулей
    StatusWorks
};

class ModuleState
{
private:
    ModuleStatus *datastatus;

    int maxModule;
public:
    ModuleState();
    ~ModuleState();

    bool SetStatus(int name, ModuleStatus status);
    void SetMaxModule(int maxmod);
    ModuleStatus GetStatus(int name);
};

}
////////////////////////////////////
#endif // APPDATA_H
