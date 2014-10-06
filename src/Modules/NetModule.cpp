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

Net::Net(NetData* net)
{
    netData = net;

    userData = new UserData[netData->maxUser];
    waitAuthUser = new int[netData->maxUser];
    currentNumberUser = 0;
}

Net::~Net()
{
    delete[] userData;
    delete[] waitAuthUser;
}

void InitNetModule(NetData& netData, app::AppMessage& dataMsg, app::AppSetting& dataSttng)
{
    app::SettingData sttngData;
    dataSttng.GetSetting(sttngData);
    netData.maxUser = sttngData.GetUserOnThread();
    netData.ratio = sttngData.GetRatioAppContAppNet();
    netData.minUser = sttngData.GetMinUserOnThread();
    netData.createThread = false;

    app::Message msg;
    msg.SetRcv(app::NewNetModule);
    app::MsgError err;
    dataMsg.GetMessage(msg, err);

    std::string bodyMsg = msg.GetBodyMsg();

    size_t first;
    size_t second;

    netData.pipe = atoi(bodyMsg.substr(first = bodyMsg.find(DATASTART)+6, second = bodyMsg.find("//")).c_str());

    bodyMsg.erase(first, (second+2)-first);

    netData.id = atoi(bodyMsg.substr(first = bodyMsg.find(DATASTART)+6, second = bodyMsg.find("//")).c_str());

    bodyMsg.erase(first, (second+2)-first);

    netData.socket = atoi(bodyMsg.substr(first = bodyMsg.find(DATASTART)+6, second = bodyMsg.find("//")).c_str());

    bodyMsg.erase(first, (second+2)-first);

    netData.numberThread = atoi(bodyMsg.substr(first = bodyMsg.find(DATASTART)+6, second = bodyMsg.find("//")).c_str());

    bodyMsg.erase(first, (second+2)-first);

    netData.currentThread = atoi(bodyMsg.substr(first = bodyMsg.find(DATASTART)+6, second = bodyMsg.find("//")).c_str());

    msg.SetRcv(netData.id);

    char buff[10];

    read(netData.pipe, buff, 10);

    dataMsg.GetMessage(msg, err);

    bodyMsg = msg.GetBodyMsg();

    netData.appControllerID = atoi(bodyMsg.substr(bodyMsg.find(DATASTART)+6, bodyMsg.find(DATAEND)).c_str());
}

bool CheckRequest(std::string bodyMsg)
{
    size_t start;
    size_t end;
    size_t previous;
    size_t second;

    start = bodyMsg.find(DATASTART);
    end = bodyMsg.rfind(DATAEND);

    if(start != std::string::npos && end != std::string::npos)
        bodyMsg.erase(start+6, end-start-6);
    else
        return false;

    //<message><nameRcv>Onito</nameRcv><nameSnd>Kira</nameSnd><event>add</event><data>good job!</data><idNode>5</idNode></message>

    previous = bodyMsg.find(MSGSTART);
    second = bodyMsg.find(MSGEND, previous);

    if (previous != bodyMsg.rfind(MSGSTART) || previous != 0 || previous == std::string::npos || second == std::string::npos || (second+10) != bodyMsg.length())
        return false;

    start = bodyMsg.find(NAMERCVSTART, previous+9);
    end = bodyMsg.find(NAMERCVEND, start);

    if (start != bodyMsg.rfind(NAMERCVSTART) || start == std::string::npos || end == std::string::npos || end != bodyMsg.rfind(NAMERCVEND))
        return false;

    previous = bodyMsg.find(NAMESNDSTART, end+10);
    second = bodyMsg.find(NAMESNDEND, previous);

    if (previous != bodyMsg.rfind(NAMESNDSTART) || previous == std::string::npos || second == std::string::npos || second != bodyMsg.rfind(NAMESNDEND))
        return false;

    start = bodyMsg.find(EVENTSTART, second+10);
    end = bodyMsg.find(EVENTEND, start);

    if (start != bodyMsg.rfind(EVENTSTART) || start == std::string::npos || end == std::string::npos || end != bodyMsg.rfind(EVENTEND))
        return false;

    previous = bodyMsg.find(DATASTART, end+8);
    second = bodyMsg.find(DATAEND, previous);

    if (previous != bodyMsg.rfind(DATASTART) || previous == std::string::npos || second == std::string::npos || second != bodyMsg.rfind(DATAEND))
        return false;

    start = bodyMsg.find(IDNODESTART, second+7);
    end = bodyMsg.find(IDNODEEND, start);

    if (start != bodyMsg.rfind(IDNODESTART) || start == std::string::npos || end == std::string::npos || end != bodyMsg.rfind(IDNODEEND))
        return false;

    return true;
}

////////////////////////////////////
}
