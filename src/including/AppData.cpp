#include "AppData.h"

namespace app {
////////////////////////////////////

AppMessage& AppData::GetMsg()
{
    return message;
}

AppSetting& AppData::GetSttng()
{
    return setting;
}

bool AppDataInit(AppData &appData)
{
   if (appData.setting.LoadSetting())
   {
       SettingData sttng;
       appData.setting.GetSetting(sttng);
       appData.message.SetThreadParam(sttng.GetCountThread());
       return true;
   }
   else if (appData.setting.ResetSetting())
   {
       SettingData sttng;
       appData.setting.GetSetting(sttng);
       appData.message.SetThreadParam(sttng.GetCountThread());
       return true;
   }

   return false;
}

////////////////////////////////////
}
