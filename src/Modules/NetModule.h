#ifndef NETMODULE_H
#define NETMODULE_H

#pragma once

#include <string>
#include <sys/poll.h>
#include <vector>
#include "../including/AppData.h"
#include <unordered_map>

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

#define QUIT "quit"

struct NetData
{
    int pipe;               // пробуждает поток в случае появления для него сообщения внутри программы  //
    int id;                 // id потока в модуле AppMessage                                            //
    int socket;             // прослушивающий сокет                                                     //
    int numberThread;       // количество потоков на текущем appcontroller
    int maxUser;            // максимальное число пользователей на поток                                //
    int ratio;              // соотношение количества потоков NetModule на AppController                //
    int appControllerId;    // id контроллера к которому привязан текущей модуль                        //
    bool createThread;      // каждый поток может создать только одного потомка, это флаг
};

struct LinkDataUser
{
    std::string name;
    int node;
};

struct LinkDataRoom
{
    std::vector<LinkDataUser> LinkUser;
};

struct UserData
{
    std::string name;

    std::string rcvBuf;
    std::string sndBuf;

    std::vector<LinkDataUser> LinkUser;
    LinkDataRoom* LinkRoom;

    int appControllerId;

    int size;
};

struct AppContData
{
    int countUser;
    int appContID;
};

struct ArrAppCont
{
    int countAppCont;
    int maxUserOnAppCont;
    int availableSpace;

    AppContData* data;

    ArrAppCont();

    void ArrInit(int ratio, int maxUser);

    int GetAppContID();
    bool CheckFreeSpace();
    void AddNewAppCont(int appID);
};

class Net
{
private:
    NetData* netData;
    UserData* userData;
    app::AppData* appData;

    pollfd *client;
    app::AppMessage &appMsg;
    std::unordered_map<std::string, int> usr;

    ArrAppCont appContData;

    char buf[1000];

    int currentNumberUser;
    int currentAppCont;

    void Handler();
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
