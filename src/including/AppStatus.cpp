#include "AppStatus.h"

namespace app {
////////////////////////////////////

ModuleState::ModuleState()
{

}

ModuleState::~ModuleState()
{
    delete[] datastatus;
}

bool ModuleState::SetStatus(int name, ModuleStatus status)
{
    if(ModuleNameFirst <= name && name < maxModule)
    {
        datastatus[name] = status;
        return true;
    }
    return false;
}

void ModuleState::SetMaxModule(int maxmod)
{
    maxModule = maxmod;
    datastatus = new ModuleStatus[maxModule];
    for(int i = 0; i< maxModule; i++)
        datastatus[i] = StatusWait;
}

ModuleStatus ModuleState::GetStatus(int name)
{
    if(ModuleNameFirst <= name && name < maxModule)
        return datastatus[name];
    return StatusUnavailable;
}

////////////////////////////////////
}




































