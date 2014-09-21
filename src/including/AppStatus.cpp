#include "AppStatus.h"

namespace app {
////////////////////////////////////

ModuleState::ModuleState()
{
    for(int i = 0; i< ModuleNameEnd; i++)
        datastatus[i] = StatusWait;
}

ModuleState::~ModuleState()
{

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
}

ModuleStatus ModuleState::GetStatus(int name)
{
    if(ModuleNameFirst <= name && name < maxModule)
        return datastatus[name];
    return StatusUnavailable;
}

////////////////////////////////////
}




































