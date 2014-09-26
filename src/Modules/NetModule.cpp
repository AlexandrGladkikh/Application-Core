#include <unistd.h>
#include "NetModule.h"
#include "../including/AppData.h"

#include <iostream>

namespace modules {
////////////////////////////////////

void* NetModule(void *appData)
{
    app::AppData *data = (app::AppData*)appData;
    app::AppMessage &dataMsg = data->GetMsg();

    app::Message msg;
    msg.SetRcv(app::NewNetModule);
    app::MsgError err;
    dataMsg.GetMessage(msg, err);

    std::cout << msg.GetBodyMsg().c_str() << std::endl;
    std::cout.flush();

    msg.SetRcv(5);

    char buff[10];

    read(4, buff, 10);

    std::cout << buff << std::endl;

    dataMsg.GetMessage(msg, err);

    std::cout << msg.GetBodyMsg().c_str() << std::endl;
    std::cout.flush();
}
////////////////////////////////////
}
