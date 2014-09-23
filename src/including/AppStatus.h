#ifndef APPDATA_H
#define APPDATA_H

#pragma once

namespace app {
////////////////////////////////////

enum ModuleName
{
    ModuleNameFirst = 0,
    controller = 0,
    moduleData = 1,
    appController = 2,
    netModule = 3,
    ModuleNameEnd = 4,
    UI = 5,
    NewModule = 6,
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
