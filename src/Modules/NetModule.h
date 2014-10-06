#ifndef NETMODULE_H
#define NETMODULE_H

#pragma once

#include <string>
#include "../including/AppData.h"

namespace modules {
////////////////////////////////////

#define MSGSTART        "<message>"
#define MSGEND          "</message>"
#define NAMERCVSTART    "<nameRcv>"
#define NAMERCVEND      "</nameRcv>"
#define NAMESNDSTART    "<nameSnd>"
#define NAMESNDEND      "</nameSnd>"
#define EVENTSTART      "<event>"
#define EVENTEND        "</event>"
#define DATASTART       "<data>"
#define DATAEND         "</data>"
#define IDNODESTART     "<idNode>"
#define IDNODEEND       "</idNode>"

struct NetData
{
    int pipe;               // пробуждает поток в случае появления для него сообщения внутри программы
    int id;                 // id потока в модуле AppMessage
    int socket;             // прослушивающий сокет
    int numberThread;       // количество потоков на текущем appcontroller
    int maxUser;            // максимальное число пользователей на поток
    int minUser;            // минимальное число пользователей на поток
    int ratio;              // количество потоков NetModule на AppController
    int appControllerID;    // id контроллера к которому привязан текущей модуль
    int currentThread;      // текущее число потоков в программе
    bool createThread;      // каждый потоко может создать только одного потомка, это флаг
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
    app::AppData* appData;

    int *waitAuthUser;
    int currentNumberUser;
public:
    Net(NetData* net, app::AppData* data);
    ~Net();
    void Process();
};

void* NetModule(void *appdata);

void InitNetModule(NetData& netData, app::AppMessage& dataMsg, app::AppSetting& dataSttng);

bool CheckRequest(std::string bodyMsg);

////////////////////////////////////
}

#endif // NETMODULE_H
