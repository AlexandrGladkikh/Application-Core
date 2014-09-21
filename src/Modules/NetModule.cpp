#include "NetModule.h"
#include "../including/AppData.h"

#include <iostream>

namespace modules {
////////////////////////////////////

void* NetModule(void *appData)
{
 /*   app::AppData *data = (app::AppData*)appData;
    app::AppMessage &dataMsg = data->GetMsg();

    app::Message msgSnd;
    app::MsgError err;

    msgSnd.CreateMessage("[nameSnd]Zack[/nameSnd][nameRcv]Onito[/nameRcv][event]addusr[/event][data]good job[/data]", app::moduleData, app::netModule);
    dataMsg.AddMessage(msgSnd, err);
    msgSnd.CreateMessage("[nameSnd]Zack[/nameSnd][nameRcv]Onito[/nameRcv][event]add[/event][data]good job[/data]", app::moduleData, app::netModule);
    dataMsg.AddMessage(msgSnd, err);
    msgSnd.CreateMessage("[nameSnd]Zack[/nameSnd][nameRcv]Onito[/nameRcv][event]del[/event][data]good job[/data]", app::moduleData, app::netModule);
    dataMsg.AddMessage(msgSnd, err);
    msgSnd.CreateMessage("[nameSnd]Zack1[/nameSnd][nameRcv]Onito[/nameRcv][event]add[/event][data]good job1[/data]", app::moduleData, app::netModule);
    dataMsg.AddMessage(msgSnd, err);
    msgSnd.CreateMessage("[nameSnd]Zack[/nameSnd][nameRcv]Onito[/nameRcv][event]read[/event][data]good job[/data]", app::moduleData, app::netModule);
    dataMsg.AddMessage(msgSnd, err);

    app::Message msgRcv;
    msgRcv.SetRcv(app::netModule);

    dataMsg.GetMessage(msgRcv, err);
    std::cout << msgRcv.GetBodyMsg().c_str() << std::endl;

    dataMsg.GetMessage(msgRcv, err);
    std::cout << msgRcv.GetBodyMsg().c_str() << std::endl;

    dataMsg.GetMessage(msgRcv, err);
    std::cout << msgRcv.GetBodyMsg().c_str() << std::endl;*/
}
////////////////////////////////////
}
