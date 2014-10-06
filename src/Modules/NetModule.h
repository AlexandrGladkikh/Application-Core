#ifndef NETMODULE_H
#define NETMODULE_H

#pragma once

#include <string>
#include "../including/AppData.h"

namespace modules {
////////////////////////////////////

#define NAMERCVSTART "<nameRcv>"
#define NAMERCVEND "</nameRcv>"
#define NAMESNDSTART "<nameSnd>"
#define NAMESNDEND "</nameSnd>"
#define EVENTSTART "<event>"
#define EVENTEND "</event>"
#define DATASTART "<data>"
#define DATAEND "</data>"
#define IDNODESTART "<idNode>"
#define IDNODEEND "</idNode>"

struct NetData
{
    int pipe;
    int id;
    int socket;
    int numberThread; // количество потоков
    int maxUser;
    int minUser;
    int ratio;
    int appControllerID; // id контроллера к которому привязан текущей модуль
    int maxThread;
    bool createThread;
};

struct UserData
{
   std::string name;
   int sock;
   bool linkData;
   char buf[1000];
};

class Net
{
private:
    NetData* netData;
    UserData* userData;
    int waitAuthUser;
public:
    Net(NetData* netData);
    void Process();
};

void* NetModule(void *appdata);

void InitNetModule(NetData& netData, app::AppMessage& dataMsg, app::AppSetting& dataSttng);

////////////////////////////////////
}

#endif // NETMODULE_H
