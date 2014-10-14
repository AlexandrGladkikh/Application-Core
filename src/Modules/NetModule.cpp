#include <unistd.h>
#include "NetModule.h"
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "../Wrap/WrapNet.h"

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

    Net net(&netData, data);

    net.Process();
}

Net::Net(NetData* net, app::AppData *data) : appMsg(data->GetMsg())
{
    netData = net;
    appData = data;

    userData = new UserData[netData->maxUser];    
    client = new pollfd[2*netData->maxUser];
    appContData.ArrInit(netData->ratio, netData->maxUser);
    appContData.AddNewAppCont(netData->appControllerId);

    currentNumberUser = 0;
    currentAppCont = 1;
}

Net::~Net()
{
    delete[] userData;
}

void Net::Process()
{
    sockaddr cliaddr;
    socklen_t clilen;
    int connfd;
    int nReady;
    int nRcv;
    int maxId = 1;

    client[0].fd = netData->socket;
    client[0].events = POLLRDNORM;
    client[1].fd = netData->pipe;
    client[1].events = POLLRDNORM;

    for (int i=2; i<netData->maxUser; i++)
        client[i].fd = -1;

    while(1)
    {
        nReady = wrap::Poll(client, maxId+1, -1);
        if (nReady == -1 && errno == EINTR)
            continue;

        if (client[0].revents & POLLRDNORM)
        {
            client[0].revents = -1;
            clilen = sizeof(cliaddr);
            while((connfd = accept(netData->socket, &cliaddr, &clilen) == -1))
                if (errno == EINTR)
                    continue;

            int val;
            const int on = 1;

            if (!wrap::Setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
            {
                wrap::Close(connfd);
                connfd = -1;
            }

            if (!wrap::Setsockopt(connfd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)))
            {
                wrap::Close(connfd);
                connfd = -1;
            }

            if ((val = wrap::Fcntl(connfd, F_GETFL, 0)) == -1)
            {
                wrap::Close(connfd);
                connfd = -1;
            }
            if ((wrap::Fcntl(connfd, F_SETFL, val | O_NONBLOCK)) == -1)
            {
                wrap::Close(connfd);
                connfd = -1;
            }

            if (connfd != -1 && (errno != EAGAIN || errno != EWOULDBLOCK || errno != ECONNABORTED))
            {
                int i;
                for (i=netData->maxUser; i<2*netData->maxUser; i++)
                {
                    if (client[i].fd == -1)
                    {
                        client[i].fd = connfd;
                        client[i].events = POLLRDNORM;
                    }
                }

                if (i == 2*netData->maxUser)
                {
                    wrap::Close(connfd);
                    client[i].fd = -1;
                }
                else
                {
                    if (i > maxId)
                        maxId = i;
                }

                ++currentNumberUser;
                if (currentNumberUser >= netData->maxUser)
                {
                    client[0].fd = -1;
                }
                else if (currentNumberUser >= appContData.availableSpace)
                {
                    client[0].fd = -1;
                }
            }

            if (--nReady<=0)
                continue;
        }

        /*
         *возможные события присылаемые внутри программы
         *1) Добавить пользователя( принять аутентификационные данные)
         *2) Удалить пользователя
         *3) Отвергнуть аутентификационные данные
         *4) Переслать указанному пользователю данные
         *5) Добавить данные LinkUser или LinkRoom (Два разных события) указанному пользователю
         *6) Завершение работы
        */

        if (client[1].revents & POLLRDNORM)
        {
            client[1].revents = -1;
            Handler();
        }
    }
}

void Net::Handler()
{
    app::Message msg;
    msg.SetRcv(netData->id);
    app::MsgError err;

    char buff[10];

    read(netData->pipe, buff, 10);

    appMsg.GetMessage(msg, err);

    if (!msg.GetBodyMsg().compare(QUIT))
    {
        msg.CreateMessage("Close Netmodule\n", app::controller, app::netModule);
        appMsg.AddMessage(msg, err);
    }
}

void InitNetModule(NetData& netData, app::AppMessage& dataMsg, app::AppSetting& dataSttng)
{
    app::SettingData sttngData;
    dataSttng.GetSetting(sttngData);
    netData.maxUser = sttngData.GetUserOnThread();
    netData.ratio = sttngData.GetRatioAppContAppNet();
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

    msg.SetRcv(netData.id);

    char buff[10];

    read(netData.pipe, buff, 10);

    dataMsg.GetMessage(msg, err);

    bodyMsg = msg.GetBodyMsg();

    netData.appControllerId = atoi(bodyMsg.substr(bodyMsg.find(DATASTART)+6, bodyMsg.find(DATAEND)).c_str());
}

bool CheckRequest(std::string bodyMsg)
{
    size_t start1;
    size_t end1;
    size_t start2;
    size_t end2;

    start1 = bodyMsg.find(DATASTART);
    end1 = bodyMsg.rfind(DATAEND);

    if(start1 != std::string::npos && end1 != std::string::npos)
        bodyMsg.erase(start1+6, end1-start1-6);
    else
        return false;

    //<message><nameRcv>Onito</nameRcv><nameSnd>Kira</nameSnd><event>add</event><data>good job!</data><idNode>5</idNode></message>

    start2 = bodyMsg.find(MSGSTART);
    end2 = bodyMsg.find(MSGEND, start2);

    if (start2 != bodyMsg.rfind(MSGSTART) || start2 != 0 || start2 == std::string::npos || end2 == std::string::npos || (end2+10) != bodyMsg.length())
        return false;

    start1 = bodyMsg.find(NAMERCVSTART, start2+9);
    end1 = bodyMsg.find(NAMERCVEND, start1);

    if (start2+9 != bodyMsg.rfind(NAMERCVSTART) || start1 == std::string::npos || end1 == std::string::npos || end1 != bodyMsg.rfind(NAMERCVEND))
        return false;

    start2 = bodyMsg.find(NAMESNDSTART, end1+10);
    end2 = bodyMsg.find(NAMESNDEND, start2);

    if (end1+10 != bodyMsg.rfind(NAMESNDSTART) || start2 == std::string::npos || end2 == std::string::npos || end2 != bodyMsg.rfind(NAMESNDEND))
        return false;

    start1 = bodyMsg.find(EVENTSTART, end2+10);
    end1 = bodyMsg.find(EVENTEND, start1);

    if (end2+10 != bodyMsg.rfind(EVENTSTART) || start1 == std::string::npos || end1 == std::string::npos || end1 != bodyMsg.rfind(EVENTEND))
        return false;

    start2 = bodyMsg.find(DATASTART, end1+8);
    end2 = bodyMsg.find(DATAEND, start2);

    if (end1+8 != bodyMsg.rfind(DATASTART) || start2 == std::string::npos || end2 == std::string::npos || end2 != bodyMsg.rfind(DATAEND))
        return false;

    start1 = bodyMsg.find(IDNODESTART, end2+7);
    end1 = bodyMsg.find(IDNODEEND, start1);

    if (end2+7 != bodyMsg.rfind(IDNODESTART) || start1 == std::string::npos || end1 == std::string::npos || end1 != bodyMsg.rfind(IDNODEEND) || end1+19 != bodyMsg.length())
        return false;

    return true;
}

ArrAppCont::ArrAppCont()
{

}

void ArrAppCont::ArrInit(int ratio, int maxUser)
{
    availableSpace = 0;

    if (ratio<0)
    {
        countAppCont = (-1)*ratio;
        maxUserOnAppCont = maxUser/countAppCont;
        data = new AppContData[((-1)*ratio)];
        for (int i=0; i<ratio; i++)
        {
            data[i].appContID = -1;
            data[i].countUser = 0;
        }
    }
    else
    {
        countAppCont = 1;
        maxUserOnAppCont = maxUser;
        data = new AppContData[1];
        data[0].appContID = -1;
        data[0].countUser = 0;
    }
}

void ArrAppCont::AddNewAppCont(int appID)
{
    for (int i=0; i<countAppCont; i++)
    {
        if (data[i].appContID == -1)
        {
            data[i].appContID = appID;
            availableSpace += maxUserOnAppCont;
            break;
        }
    }
}
int ArrAppCont::GetAppContID()
{
    for (int i=0; i<countAppCont; i++)
    {
        if (data[i].appContID != -1 && data[i].countUser < maxUserOnAppCont)
        {
            data[i].countUser++;
            return data[i].appContID;
        }
    }

    return -1;
}

bool ArrAppCont::CheckFreeSpace()
{
    if (availableSpace != 0)
        return true;
    else
        return false;
}

////////////////////////////////////
}
