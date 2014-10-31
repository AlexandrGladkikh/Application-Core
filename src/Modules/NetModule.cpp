#include <unistd.h>
#include "NetModule.h"
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "../Wrap/WrapNet.h"
#include "../Modules/AppController.h"

namespace modules {
////////////////////////////////////

void* NetModule(void *appData)
{
    app::AppData *data = (app::AppData*)appData;
    app::AppMessage &dataMsg = data->GetMsg();
    app::AppSetting dataSttng;
    dataSttng = data->GetSttng();

    NetData netData;

    if (InitNetModule(netData, dataMsg, dataSttng))
    {
        Net net(&netData, data);
        net.Process();
    }
}

Net::Net(NetData* net, app::AppData *data) : appMsg(data->GetMsg())
{
    netData = net;
    appData = data;

    userData = new UserData[netData->maxUser];    
    client = new pollfd[2*netData->maxUser];

    currentNumberUser = 0;
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
                }
                else
                {
                    if (i > maxId)
                        maxId = i;
                }

                ++currentNumberUser;
                if (currentNumberUser >= netData->maxUser)
                {
                    CreateNewThread();
                }

                if (--nReady<=0)
                    continue;
            }
        }

        /*
         *возможные события присылаемые внутри программы
         *1) Добавить пользователя( принять аутентификационные данные)
         * //2) Удалить пользователя
         *3) Отвергнуть аутентификационные данные
         *4) Переслать указанному пользователю данные
         *5) Добавить данные LinkUser или LinkRoom (Два разных события) указанному пользователю
         *6) Завершение работы
        */

        if (client[1].revents & POLLRDNORM)
        {
            if (!Handler())
                break;
        }
    }
}

bool Net::Handler()
{
    app::Message msg;
    msg.SetRcv(netData->id);
    app::MsgError err;

    char buff[10];

    read(netData->pipe, buff, 4);

    appMsg.GetMessage(msg, err);

    if (!msg.GetBodyMsg().compare(QUIT) && msg.GetSnd() == app::controller)
    {
        msg.CreateMessage("Close Netmodule\n", app::controller, app::netModule);
        appMsg.AddMessage(msg, err);
    }
    else
    {
        appMsg.DeleteModule(netData->id);
    }

    return false;
}

void Net::CreateNewThread()
{
    if (netData->createThread == false)
    {
        std::string bodyMsg;
        app::Message event;
        app::MsgError err;

        int modID[2];
        appMsg.AddNewModule(&modID[0], &modID[1]);

        if (modID[1] != -1)
        {
            if (netData->ratio > netData->numberThread)
            {
                std::string bodyMsg;
                app::Message event;
                app::MsgError err;
                char buff[10];
                sprintf(buff, "%d", modID[0]);  // pipe для ожидания сообщения в select
                bodyMsg.append(buff);
                bodyMsg.append("//");
                sprintf(buff, "%d", modID[1]);  // id потока по которому будут запрашиваться сообщения в appmessage
                bodyMsg.append(buff);
                bodyMsg.append("//");
                sprintf(buff, "%d", netData->socket);    // прослушивающий сокет
                bodyMsg.append(buff);
                bodyMsg.append("//");
                sprintf(buff, "%d", (netData->numberThread+1));         // количество потоков в данный момент на appcontroller
                bodyMsg.append(buff);
                bodyMsg.append(DATAEND);
                event.CreateMessage(bodyMsg.c_str(), app::NewNetModule, app::UI);
                appMsg.AddMessage(event, err);

                pthread_attr_t attr;

                pthread_t threadNet;

                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

                pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

                pthread_attr_setschedpolicy(&attr, SCHED_RR);

                pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);

                pthread_attr_init(&attr);

                pthread_create(&threadNet, &attr, NetModule, appData);

                char buf[10];
                sprintf(buf, "%d", netData->appControllerId);
                bodyMsg.append(buf);

                event.CreateMessage(bodyMsg.c_str(), modID[1], netData->appControllerId);

                appMsg.AddMessage(event, err);

                pthread_attr_destroy(&attr);
            }
            else
            {
                int id;
                appMsg.AddNewModule(&id);

                char buff[10];
                sprintf(buff, "%d", modID[0]);  // pipe для ожидания сообщения в select
                bodyMsg.append(buff);
                bodyMsg.append("//");
                sprintf(buff, "%d", modID[1]);  // id потока по которому будут запрашиваться сообщения в appmessage
                bodyMsg.append(buff);
                bodyMsg.append("//");
                sprintf(buff, "%d", netData->socket);    // прослушивающий сокет
                bodyMsg.append(buff);
                bodyMsg.append("//");
                sprintf(buff, "%d", (netData->numberThread+1));         // количество потоков в данный момент на appcontroller
                bodyMsg.append(buff);
                bodyMsg.append(DATAEND);
                event.CreateMessage(bodyMsg.c_str(), app::NewNetModule, app::UI);
                appMsg.AddMessage(event, err);

                pthread_attr_t attr;

                pthread_t threadNet;

                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

                pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

                pthread_attr_setschedpolicy(&attr, SCHED_RR);

                pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);

                pthread_attr_init(&attr);

                pthread_create(&threadNet, &attr, NetModule, appData);

                bodyMsg.erase();
                sprintf(buff, "%d", id);
                bodyMsg.append(buff);
                bodyMsg.append(DATAEND);

                event.CreateMessage(bodyMsg.c_str(), app::NewAppModule, app::UI);

                appMsg.AddMessage(event, err);

                bodyMsg.erase();
                sprintf(buff, "%d", modID[1]);
                bodyMsg.append(buff);
                bodyMsg.append(DATAEND);

                event.CreateMessage(bodyMsg.c_str(), app::NewAppModule, app::UI);

                appMsg.AddMessage(event, err);

                pthread_t threadCont;

                pthread_create(&threadCont, &attr, modules::AppController, appData);

                pthread_attr_destroy(&attr);
            }
        }
    }

    client[0].fd = -1;
    netData->createThread = true;
}

bool InitNetModule(NetData& netData, app::AppMessage& dataMsg, app::AppSetting& dataSttng)
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
    if (msg.GetSnd() == app::controller)
    {
        msg.CreateMessage("Close NetModule\n", app::controller, app::NewNetModule);
        dataMsg.AddMessage(msg, err);
        return false;
    }

    std::string bodyMsg = msg.GetBodyMsg();

    size_t second;

    netData.pipe = atoi(bodyMsg.substr(0, second = bodyMsg.find("//")).c_str());

    bodyMsg.erase(0, (second+2));

    netData.id = atoi(bodyMsg.substr(0, second = bodyMsg.find("//")).c_str());

    bodyMsg.erase(0, (second+2));

    netData.socket = atoi(bodyMsg.substr(0, second = bodyMsg.find("//")).c_str());

    bodyMsg.erase(0, (second+2));

    netData.numberThread = atoi(bodyMsg.c_str());

    msg.SetRcv(netData.id);

    char buff[10];

    read(netData.pipe, buff, 4);

    dataMsg.GetMessage(msg, err);

    if (msg.GetSnd() == app::controller)
    {
        msg.CreateMessage("Close NetModule\n", app::controller, app::NewNetModule);
        dataMsg.AddMessage(msg, err);
        return false;
    }

    bodyMsg = msg.GetBodyMsg();

    netData.appControllerId = atoi(bodyMsg.c_str());

    if (netData.appControllerId == 0)
        return false;

    return true;
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

////////////////////////////////////
}
