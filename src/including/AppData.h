#ifndef APPINIT_H
#define APPINIT_H

#pragma once

#include "../including/AppMessage.h"
#include "../including/AppSettings.h"

namespace app {
////////////////////////////////////

struct AppData
{
    AppMessage message;
    AppSetting setting;

    AppMessage& GetMsg();
    AppSetting& GetSttng();
};

bool AppDataInit(AppData& appData);

////////////////////////////////////
}

#endif // APPINIT_H
