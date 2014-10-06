#include <unistd.h>
#include "NetModule.h"

#include <iostream>

namespace modules {
////////////////////////////////////

void* NetModule(void *appData)
{
    app::AppData *data = (app::AppData*)appData;
    app::AppMessage &dataMsg = data->GetMsg();
    app::AppSetting dataSttng;
    dataSttng = data->GetSttng();

    NetData netData;

    InitNetModule(netData, dataMsg, dataSttng);
}

void InitNetModule(NetData& netData, app::AppMessage& dataMsg, app::AppSetting& dataSttng)
{
    app::SettingData sttngData;
    dataSttng.GetSetting(sttngData);
    netData.maxUser = sttngData.GetUserOnThread();
    netData.ratio = sttngData.GetRatioAppContAppNet();
    netData.minUser = sttngData.GetMinUserOnThread();
    netData.maxThread = sttngData.GetMaxThread();
    netData.createThread = false;

    app::Message msg;
    msg.SetRcv(app::NewNetModule);
    app::MsgError err;
    dataMsg.GetMessage(msg, err);

    std::string bodyMsg = msg.GetBodyMsg();

    size_t first;
    size_t second;

    std::cout << bodyMsg.c_str() << std::endl;

    netData.pipe = atoi(bodyMsg.substr(first = bodyMsg.find(DATASTART)+6, second = bodyMsg.find("//")).c_str());

    std::cout << netData.pipe << std::endl;

    bodyMsg.erase(first, (second+2)-first);

    netData.id = atoi(bodyMsg.substr(first = bodyMsg.find(DATASTART)+6, second = bodyMsg.find("//")).c_str());

    std::cout << netData.id << std::endl;

    bodyMsg.erase(first, (second+2)-first);

    netData.socket = atoi(bodyMsg.substr(first = bodyMsg.find(DATASTART)+6, second = bodyMsg.find("//")).c_str());

    std::cout << netData.socket << std::endl;

    bodyMsg.erase(first, (second+2)-first);

    netData.numberThread = atoi(bodyMsg.substr(first = bodyMsg.find(DATASTART)+6, second = bodyMsg.find("//")).c_str());

    std::cout << netData.numberThread << std::endl;

    bodyMsg.erase(first, (second+2)-first);

    netData.maxThread = atoi(bodyMsg.substr(first = bodyMsg.find(DATASTART)+6, second = bodyMsg.find("//")).c_str());

    std::cout << netData.maxThread << std::endl;

    msg.SetRcv(netData.id);

    char buff[10];

    read(4, buff, 10);

    dataMsg.GetMessage(msg, err);

    bodyMsg = msg.GetBodyMsg();

    netData.appControllerID = atoi(bodyMsg.substr(bodyMsg.find(DATASTART)+6, bodyMsg.find(DATAEND)).c_str());

    std::cout << netData.appControllerID << std::endl;
}

////////////////////////////////////
}
