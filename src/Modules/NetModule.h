#ifndef NETMODULE_H
#define NETMODULE_H

#pragma once

#include <string>
#include <sys/poll.h>
#include <vector>
#include "../including/AppData.h"
#include <unordered_map>
#include <deque>
#include "ModulesDefined.h"

namespace modules {
////////////////////////////////////

struct NetData
{
    int pipe;               // пробуждает поток в случае появления для него сообщения внутри программы  //
    int id;                 // id потока в модуле AppMessage                                            //
    int socket;             // прослушивающий сокет                                                     //
    int numberThread;       // количество потоков на текущем appcontroller                              //
    int maxUser;            // максимальное число пользователей на поток                                //
    int ratio;              // соотношение количества потоков NetModule на AppController                //
    int appControllerId;    // id контроллера к которому привязан текущей модуль                        //
    int userOnchatRoom;     // число пользователей на одну комнату чата
    bool createThread;      // каждый поток может создать только одного потомка, это флаг               //
};

struct UserData
{
    std::string name;

    std::string sndBuf;

    int numberRoom;
    int posInRoom;
};

struct PrivateMsg
{
    int idUserSnd;
    int idUserRcv;

    std::string msg;
};

struct PublicMsg
{
    int idUser;

    std::string msg;
};

struct ChatRoom
{
    unsigned int *usrID;
    unsigned int availableSpace;
    unsigned int numUsr;
    bool waitHandler;

    std::deque<PrivateMsg> queuePrivateMsg;

    std::deque<PublicMsg> queuePublicMsg;

    ChatRoom() : waitHandler(false) { }
    ~ChatRoom();

    void ChatRoomInit(int valUsr);

    void AddPrivateMsg(PrivateMsg msg);
    void AddPublicMsg(PublicMsg msg);

    std::deque<PrivateMsg>::iterator GetPrivateMsg(int& size);
    std::deque<PublicMsg>::iterator GetPublicMsg(int& size);
    void ErasePublicMsg(std::deque<PublicMsg>::iterator itPublicMsg);
    void ErasePrivateMsg(std::deque<PrivateMsg>::iterator itPrivateMsg);
    bool CheckEmptyMsg();

    unsigned int* GetUsrID();
    unsigned int GetUsr(int pos);

    bool CheckAvailableSpace();
    bool CheckWaitHandler();
    void SetWaitHandler(bool status);

    unsigned int AddUsr(int id);
    void RemoveUsr(unsigned int pos);
};

class Chat
{
private:
    ChatRoom *chatRoom;
    unsigned int numRoom;

    std::deque<int> roomWaitHandler;
public:
    Chat(int valRoom, int numUsrOnRoom);
    ~Chat();

     void AddUsr(int id, unsigned int& posInRoom, unsigned int& numberRoom, const char* name, int maxUser, UserData *usrData);
     void RemoveUsr(unsigned int posInRoom, unsigned int numberRoom);

     std::deque<int>::iterator GetIDRoomWaitHadler(int &size);
     ChatRoom* GetChatRoom(unsigned int num);
     void SetRoomWaitHandler(unsigned int num);
     void RemoveRoomWaitHandler(std::deque<int>::iterator room);
};

class Net
{
private:
    NetData* netData;
    UserData* userData;
    app::AppData* appData;
    Chat *chat;

    pollfd *client;
    app::AppMessage &appMsg;

    char buf[1000];

    int currentNumberUser;

    int maxId;

    bool HandlerLocalMsg();
    void HandlerNewUser();
    void SendData();
    void RecvData();
public:
    Net(NetData* net, app::AppData* data);
    ~Net();
    void Process();
    void CreateNewThread();
};

void* NetModule(void *appdata);

bool InitNetModule(NetData& netData, app::AppMessage& dataMsg, app::AppSetting& dataSttng);

bool CheckRequest(std::string bodyMsg);

////////////////////////////////////
}

#endif // NETMODULE_H
