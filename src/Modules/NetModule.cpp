#include <unistd.h>
#include "NetModule.h"
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "../Wrap/WrapNet.h"
#include "../Wrap/UserData.h"
#include "../Modules/AppController.h"

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

    chat = new Chat(((netData->maxUser)/netData->userOnchatRoom), netData->userOnchatRoom);
    userData = new UserData[netData->maxUser+2];
    client = new pollfd[2*netData->maxUser+2];
    for (int i=0; i<2*netData->maxUser+2; i++)
    {
        client[i].fd = -1;
    }

    //netData->maxUser += 2;

    currentNumberUser = 0;
}

Net::~Net()
{
    delete[] userData;
    delete[] client;
    delete chat;
}

void Net::Process()
{
    sockaddr_storage cliaddr;
    socklen_t clilen;
    int connfd;
    int nReady;
    maxId = 1;

    client[0].fd = netData->socket;
    client[0].events = POLLRDNORM;
    client[1].fd = netData->pipe;
    client[1].events = POLLRDNORM;

    while(1)
    {
        std::cout << "\nначало цикла" << std::endl;
        std::cout.flush();

        nReady = wrap::Poll(client, maxId+1, -1);
        if (nReady == -1 || errno == EINTR)
        {
            std::cout << "ошибка poll: " << errno << std::endl;
            std::cout.flush();
            continue;
        }

        for (int i=0; i<2*netData->maxUser+2; i++)
        {
            if (client[i].fd != -1)
            {
                std::cout << i << ' ' << client[i].fd << std::endl;
                std::cout.flush();
            }

        }

        std::cout << "успешное завершение poll, число готовых дескрипторов: " << nReady << std::endl;
        std::cout.flush();

        if (client[0].revents & POLLRDNORM)
        {
            std::cout << "новый пользователь" << std::endl;
            std::cout.flush();
            clilen = sizeof(cliaddr);
            while((connfd = accept(client[0].fd, (sockaddr* ) &cliaddr, &clilen)) == -1)
                if (errno == EINTR)
                    continue;
                else if (errno == EAGAIN)
                    break;


            std::cout << "приняли нового пользователя с сокетом:" << ' ' << connfd << std::endl;
            std::cout.flush();

            int val;
            const int on = 1;

            if (!wrap::Setsockopt(connfd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)))
            {
                std::cout << "неудалось установить параметры сокета" << std::endl;
                std::cout.flush();
                wrap::Close(connfd);
                connfd = -1;
            }

            if ((val = wrap::Fcntl(connfd, F_GETFL, 0)) == -1 && connfd != -1)
            {
                std::cout << "неудалось установить параметры сокета" << std::endl;
                std::cout.flush();
                wrap::Close(connfd);
                connfd = -1;
            }
            if ((wrap::Fcntl(connfd, F_SETFL, val | O_NONBLOCK)) == -1 && connfd != -1)
            {
                std::cout << "неудалось установить параметры сокета" << std::endl;
                std::cout.flush();
                wrap::Close(connfd);
                connfd = -1;
            }

            if (connfd != -1 && (errno != EAGAIN || errno != EWOULDBLOCK || errno != ECONNABORTED))
            {
                if (currentNumberUser < netData->maxUser)
                {
                    int i;
                    for (i=netData->maxUser+2; i<2*netData->maxUser+2; i++)
                    {
                        if (client[i].fd == -1)
                        {
                            std::cout << "id нового клиента: " << i << std::endl;
                            std::cout.flush();
                            client[i].fd = connfd;
                            client[i].events = POLLRDNORM;// & POLLRDHUP;
                            break;
                        }
                    }

                    if (i == 2*netData->maxUser+2 && connfd != -1)
                    {
                        std::cout << "перебор пользователей, текущее число: " << currentNumberUser << std::endl;
                        std::cout.flush();
                        wrap::Close(connfd);
                        connfd = -1;
                    }
                    else
                    {
                        ++currentNumberUser;
                        if (i > maxId)
                            maxId = i;
                    }

                    /*if (currentNumberUser >= netData->maxUser)
                    {
                        std::cout << "создание нового потока" << std::endl;
                        std::cout.flush();
                        CreateNewThread();
                    }*/

                    if (--nReady<=0)
                        continue;
                }
                else
                {
                    std::cout << "перебор пользователей, текущее число: " << currentNumberUser
                              << "допустимое число пользователей: " << netData->maxUser << std::endl;
                    std::cout.flush();
                    wrap::Close(connfd);
                    connfd = -1;
                }
            }
        }

        /*
         *возможные события присылаемые внутри программы
         *1) Добавить пользователя( принять аутентификационные данные)
         *2) Отвергнуть аутентификационные данные
         *3) Завершение работы
        */

        if (client[1].revents & POLLRDNORM)
        {
            if (!HandlerLocalMsg())
                break;
        }

        HandlerNewUser();

        RecvData();

        SendData();
    }
}

bool Net::HandlerLocalMsg()
{
    app::Message msg;
    msg.SetRcv(netData->id);
    app::MsgError err;

    char buff[10];
    ssize_t readSize;

    while ((readSize = read(netData->pipe, buff, 4)) == -1)
    { }

    appMsg.GetMessage(msg, err);

    std::cout << "получили локальное сообщение" << std::endl;
    std::cout.flush();

    const char* textMsg = msg.GetBodyMsg();
    std::string bodyMsg = textMsg;

    if (!bodyMsg.compare(QUIT) && msg.GetSnd() == app::controller)
    {
        for (int i=0; i<2*netData->maxUser+2; i++)
        {
            if (client[i].fd != -1)
            wrap::Close(client[i].fd);
            currentNumberUser--;
        }
        msg.CreateMessage("Close Netmodule\n", app::controller, app::netModule);
        appMsg.AddMessage(msg, err);

        return false;
    }

    //std::string& bodyMsg= msg.GetBodyMsg();
    wrap::UserDataAdd *usrData = (wrap::UserDataAdd*)textMsg;

    std::cout << "данные пользователя корректны или нет?(1/0)" << usrData->CheckFlag() << std::endl;
    std::cout.flush();

    if (usrData->CheckFlag())
    {
        for (int i=2; i<netData->maxUser+2; i++)
        {
            if (client[i].fd == -1)
            {
                std::cout << "данные корректны" << std::endl;
                std::cout.flush();
                std::cout << "аутентификационные данные: логин-" << usrData->GetName() <<
                          " пароль-" << usrData->GetPass() << "сокет-" << usrData->GetSock() << std::endl;
                std::cout.flush();
                client[i].fd = usrData->GetSock();
                //client[i].fd = client[usrData->GetPosition()].fd;
                client[i].events = POLLRDNORM;
                client[usrData->GetPosition()].fd = -1;
                if (usrData->GetPosition() > maxId)
                    maxId = usrData->GetPosition();

                unsigned int posInRoom;
                unsigned int numberRoom;

                std::cout << "добавление пользователя в чат" << std::endl;
                std::cout.flush();

                chat->AddUsr(i, posInRoom, numberRoom, usrData->GetName(), netData->userOnchatRoom, userData);

                userData[i].numberRoom = numberRoom;
                userData[i].posInRoom = posInRoom;

                userData[i].name = usrData->GetName();

                //exit(0);

                break;
            }
        }
    }
    else
    {
        wrap::Close(usrData->GetSock());
        std::cout << "данные не корректны" << std::endl;
        currentNumberUser--;
    }

    std::cout << "завершили обработку локальных сообщений" << std::endl;
    std::cout.flush();

    return true;
}

void Net::HandlerNewUser()
{
    ssize_t nRcv;
    for (int i=netData->maxUser+2; i<2*netData->maxUser+2; i++)
    {
        if (client[i].fd != -1 && client[i].revents & (POLLRDNORM | POLLERR | POLLRDHUP | POLLHUP))
        {
            std::cout << "Чтение данных от нового пользователя с id^: " << client[i].fd << std::endl;
            std::cout.flush();
            while (1)
            {
                if ((nRcv = read(client[i].fd, buf, 1000)) == -1)
                {
                    std::cout << "Чтение данных от нового пользователя с id^: " << client[i].fd << std::endl;
                    std::cout.flush();

                    if (errno == EAGAIN)
                        break;
                    else if (errno == EINTR)
                        continue;
                    else
                    {
                        std::cout << "закрытие сокета: " << client[i].fd << std::endl;
                        std::cout.flush();
                        wrap::Close(client[i].fd);
                        client[i].fd = -1;
                        currentNumberUser--;
                        break;
                    }
                }
                else if (nRcv == 0)
                {
                    std::cout << "закрытие сокета: " << client[i].fd << std::endl;
                    std::cout.flush();
                    wrap::Close(client[i].fd);
                    client[i].fd = -1;
                    currentNumberUser--;
                    break;
                }
                else if (nRcv > 0)
                {
                    std::cout << "Обработка данных от нового пользователя" << std::endl;
                    std::cout.flush();
                    char usrDat[100];
                    wrap::UserDataAdd *usrData = (wrap::UserDataAdd*) usrDat;
                    usrData->SetSock(client[i].fd);
                    usrData->SetPosition(i);
                    std::string strRequest;
                    strRequest.append(buf);
                    size_t loginStart = strRequest.find(LOGINSTART);
                    size_t loginEnd = strRequest.find(LOGINEND, loginStart);
                    size_t passStart = strRequest.find(PASSSTART, loginEnd);
                    size_t passEnd = strRequest.find(PASSEND, passStart);

                    if (loginStart != std::string::npos && loginEnd != std::string::npos && passStart != std::string::npos && passEnd != std::string::npos)
                    {
                        std::string login = strRequest.substr((loginStart+7), (loginEnd-(loginStart+7)));
                        std::string pass = strRequest.substr((loginEnd+14), (passEnd-(loginEnd+14)));

                        std::cout << "Отправа аутентификационных данных: логин-" << login.c_str() <<
                                  " пароль-" << pass.c_str() << " сокет: " << usrData->GetSock() << std::endl;
                        std::cout.flush();

                        usrData->SetName(login.c_str());
                        usrData->SetPass(pass.c_str());

                        app::Message msg;

                        msg.CreateMessage(usrDat, netData->appControllerId, netData->id);
                        app::MsgError err;
                        appMsg.AddMessage(msg, err);
                    }
                    else
                    {
                        std::cout << "некорректные аутентификационные данные" << buf << std::endl;
                        std::cout.flush();
                        wrap::Close(client[i].fd);
                        client[i].fd = -1;
                        currentNumberUser--;
                    }

                    break;
                }
            }
        }
    }
}

void Net::RecvData()
{
    ssize_t nRcv;

    for (int i=2; i<netData->maxUser+2; i++)
    {
        if (client[i].fd != -1 && client[i].revents & (POLLRDNORM | POLLERR | POLLRDHUP | POLLHUP))
        {
            std::cout << "Чтение данных от пользователя с id: " << client[i].fd << std::endl;
            std::cout.flush();
            while (1)
            {
                if ((nRcv = read(client[i].fd, buf, 1000)) == -1)
                {
                    if (errno == EAGAIN)
                        break;
                    else if (errno == EINTR)
                        continue;
                    else
                    {
                        std::cout << "закрытие сокета: " << client[i].fd << std::endl;
                        std::cout.flush();
                        wrap::Close(client[i].fd);
                        client[i].fd = -1;
                        chat->RemoveUsr(userData[i].posInRoom, userData[i].numberRoom, userData[i].name.c_str());
                        currentNumberUser--;
                        break;
                    }
                }
                else if (nRcv == 0)
                {
                    std::cout << "закрытие сокета: " << client[i].fd << std::endl;
                    std::cout.flush();
                    wrap::Close(client[i].fd);
                    std::string name = userData[i].name + '<' + std::to_string(userData[i].posInRoom)+ '>';
                    chat->RemoveUsr(userData[i].posInRoom, userData[i].numberRoom, name.c_str());
                    client[i].fd = -1;
                    currentNumberUser--;
                    break;
                }
                else if (nRcv > 0)
                {
                    std::string strRequest;
                    strRequest.append(buf, nRcv);
                    std::cout << "получено сообщение: " << strRequest << std::endl;
                    std::cout.flush();

                    ChatRoom *chatRoom;
                    chatRoom = chat->GetChatRoom(userData[i].numberRoom);

                    if (CheckRequest(strRequest))
                    {
                        int rcv = atoi(strRequest.substr((strRequest.find("<nameRcv>")+9), strRequest.find("</nameRcv>")).c_str());
                        if (rcv == -1)
                        {
                            std::cout << "публичное сообщение" << std::endl;
                            std::cout.flush();
                            PublicMsg pubMsg;
                            pubMsg.idUser = userData[i].posInRoom;

                            size_t dataStart = strRequest.find(DATASTART)+6;
                            size_t dataEnd = strRequest.rfind(DATAEND);
                            pubMsg.msg.append(PRIVATEMSGSTART);
                            char baf[10];
                            sprintf(baf, "%d", -1);
                            pubMsg.msg.append(baf);
                            pubMsg.msg.append(PRIVATEMSGEND);
                            pubMsg.msg.append(NAMESNDSTART);
                            pubMsg.msg.append(userData[i].name + "<" + std::to_string(userData[i].posInRoom) + ">");
                            pubMsg.msg.append(NAMESNDEND);
                            pubMsg.msg.append(DATASTART);
                            pubMsg.msg.append(strRequest.substr(dataStart, (dataEnd-dataStart)));
                            pubMsg.msg.append(DATAEND);
                            chatRoom->AddPublicMsg(pubMsg);

                            if (!chatRoom->CheckWaitHandler())
                            {
                                std::cout << "добавление комнаты в очередь ожидающих обработки, номер комнаты: " << userData[i].numberRoom << std::endl;
                                std::cout.flush();
                                chat->SetRoomWaitHandler(userData[i].numberRoom);
                                chatRoom[i].SetWaitHandler(true);
                            }
                        }
                        else if (rcv >= 0 && rcv < netData->userOnchatRoom)
                        {
                            std::cout << "приватное сообщение" << std::endl;
                            std::cout.flush();

                            if (chatRoom->GetUsr(rcv) != -1)
                            {
                                PrivateMsg prvtMsg;
                                prvtMsg.idUserRcv = rcv;
                                prvtMsg.idUserSnd = userData[i].posInRoom;
                                size_t dataStart = strRequest.find(DATASTART)+6;
                                size_t dataEnd = strRequest.rfind(DATAEND);
                                prvtMsg.msg.append(PRIVATEMSGSTART);
                                char baf[10];
                                sprintf(baf, "%d", 1);
                                prvtMsg.msg.append(baf);
                                prvtMsg.msg.append(PRIVATEMSGEND);
                                prvtMsg.msg.append(NAMESNDSTART);
                                prvtMsg.msg.append(userData[i].name + "<" + std::to_string(userData[i].posInRoom) + ">");
                                prvtMsg.msg.append(NAMESNDEND);
                                prvtMsg.msg.append(DATASTART);
                                prvtMsg.msg.append(strRequest.substr(dataStart, (dataEnd-dataStart)));
                                prvtMsg.msg.append(DATAEND);
                                chatRoom->AddPrivateMsg(prvtMsg);

                                if (!chatRoom->CheckWaitHandler())
                                {
                                    std::cout << "добавление комнаты в очередь ожидающих обработки, номер комнаты: " << userData[i].numberRoom << std::endl;
                                    std::cout.flush();
                                    chat->SetRoomWaitHandler(userData[i].numberRoom);
                                    chatRoom[i].SetWaitHandler(true);
                                }
                            }
                            else
                            {
                                PrivateMsg prvtMsg;
                                prvtMsg.idUserRcv = userData[i].posInRoom;
                                prvtMsg.idUserSnd = -1;
                                prvtMsg.msg.append(MSGSTART);
                                prvtMsg.msg.append(BADREQUEST);
                                prvtMsg.msg.append(MSGEND);
                                chatRoom->AddPrivateMsg(prvtMsg);

                                if (!chatRoom->CheckWaitHandler())
                                {
                                    std::cout << "добавление комнаты в очередь ожидающих обработки, номер комнаты: " << userData[i].numberRoom << std::endl;
                                    std::cout.flush();
                                    chat->SetRoomWaitHandler(userData[i].numberRoom);
                                    chatRoom[i].SetWaitHandler(true);
                                }
                            }
                        }

                    }
                    else
                    {
                        std::cout << "некорректное выражение" << std::endl;
                        std::cout.flush();
                        PrivateMsg prvtMsg;
                        prvtMsg.idUserRcv = userData[i].posInRoom;
                        prvtMsg.idUserSnd = -1;
                        prvtMsg.msg.append(MSGSTART);
                        prvtMsg.msg.append(BADREQUEST);
                        prvtMsg.msg.append(MSGEND);
                        chatRoom->AddPrivateMsg(prvtMsg);

                        if (!chatRoom->CheckWaitHandler())
                        {
                            std::cout << "добавление комнаты в очередь ожидающих обработки, номер комнаты: " << userData[i].numberRoom << std::endl;
                            std::cout.flush();
                            chat->SetRoomWaitHandler(userData[i].numberRoom);
                            chatRoom[i].SetWaitHandler(true);
                        }
                    }

                    break;
                }
            }
        }
    }
}

void Net::SendData()
{
    int sizeWaitHandler;
    std::deque<int>::iterator itWaitRoom = chat->GetIDRoomWaitHadler(sizeWaitHandler);

    std::cout << "число комнат ожидающих обработки: " << sizeWaitHandler << std::endl;

    for (int i=0; i<sizeWaitHandler; i++)
    {
        std::cout << "номер комнаты ожидающей обработки: " << *(itWaitRoom+i) << std::endl;
    }

    ChatRoom *room;
    int nSnd;
    int sizePrvtMsg;
    PrivateMsg prvtMsg;
    std::deque<PrivateMsg>::iterator itPrvtMsg;
    PublicMsg pubMsg;
    int sizePublicMsg;
    std::deque<PublicMsg>::iterator itPublicMsg;
    unsigned int* usrInRoom;

    for (int i=0; i<sizeWaitHandler; i++)
    {
        room = chat->GetChatRoom(*itWaitRoom);

        std::cout << "отправка данных пользователям" << std::endl;
        std::cout.flush();

        itPrvtMsg = room->GetPrivateMsg(sizePrvtMsg);

        std::cout << "число приватных сообщений: " << sizePrvtMsg << std::endl;

        for (int i=0; i<sizePrvtMsg; i++)
        {
            prvtMsg = *itPrvtMsg;

            std::cout << "текст приватного сообщения: " << prvtMsg.msg.c_str() << std::endl;

            if (client[room->GetUsr(prvtMsg.idUserRcv)].fd != -1)
            while (1)
            {
                std::cout << "сокет пользователя получающего сообщение: " << client[room->GetUsr(prvtMsg.idUserRcv)].fd << std::endl;
                userData[room->GetUsr(prvtMsg.idUserRcv)].sndBuf.append(prvtMsg.msg);
                room->ErasePrivateMsg(itPrvtMsg);

                nSnd = write(client[room->GetUsr(prvtMsg.idUserRcv)].fd, userData[room->GetUsr(prvtMsg.idUserRcv)].sndBuf.c_str(), userData[room->GetUsr(prvtMsg.idUserRcv)].sndBuf.length());
                std::cout << "передано данных " << nSnd << " всего данных " << userData[room->GetUsr(prvtMsg.idUserRcv)].sndBuf.length() << std::endl;

                if (nSnd == -1 && errno == EINTR)
                    continue;
                else if (nSnd == -1 && errno == EAGAIN)
                    client[room->GetUsr(prvtMsg.idUserRcv)].events = client[room->GetUsr(prvtMsg.idUserRcv)].events | POLLOUT;
                else if (nSnd == -1 && errno == EPIPE)
                {
                    chat->RemoveUsr(userData[room->GetUsr(prvtMsg.idUserRcv)].posInRoom, userData[room->GetUsr(prvtMsg.idUserRcv)].numberRoom, userData[i].name.c_str());
                    wrap::Close(client[room->GetUsr(prvtMsg.idUserRcv)].fd);
                    client[room->GetUsr(prvtMsg.idUserRcv)].fd = -1;
                    currentNumberUser--;
                }
                else if (nSnd == userData[room->GetUsr(prvtMsg.idUserRcv)].sndBuf.length())
                {
                    if (client[room->GetUsr(prvtMsg.idUserRcv)].events & POLLOUT)
                        client[room->GetUsr(prvtMsg.idUserRcv)].events ^ POLLOUT;
                    userData[room->GetUsr(prvtMsg.idUserRcv)].sndBuf.clear();
                }
                else if (nSnd < userData[room->GetUsr(prvtMsg.idUserRcv)].sndBuf.length())
                {
                    client[room->GetUsr(prvtMsg.idUserRcv)].events = client[room->GetUsr(prvtMsg.idUserRcv)].events | POLLOUT;
                    userData[room->GetUsr(prvtMsg.idUserRcv)].sndBuf.erase(nSnd);
                }

                break;
            }

            itPrvtMsg++;
        }

        itPublicMsg = room->GetPublicMsg(sizePublicMsg);

        std::cout << "число общих сообщений: " << sizePublicMsg << std::endl;

        for (int i=0; i<sizePublicMsg; i++)
        {
            pubMsg = *itPublicMsg;

            std::cout << "текст общего сообщения: " << pubMsg.msg.c_str() << std::endl;
            usrInRoom = room->GetUsrID();

            for (int i=0; i<netData->userOnchatRoom; i++)
            {
                if (usrInRoom[i] != -1 && i != pubMsg.idUser)
                    while (1)
                    {
                        userData[usrInRoom[i]].sndBuf.append(pubMsg.msg);

                        nSnd = write(client[usrInRoom[i]].fd, userData[usrInRoom[i]].sndBuf.c_str(), userData[usrInRoom[i]].sndBuf.length());

                        if (nSnd == -1 && errno == EINTR)
                            continue;
                        if (nSnd == -1 && errno == EAGAIN)
                            client[usrInRoom[i]].events = client[usrInRoom[i]].events | POLLOUT;
                        else if (nSnd == -1 && errno == EPIPE)
                        {
                            chat->RemoveUsr(userData[usrInRoom[i]].posInRoom, userData[usrInRoom[i]].numberRoom, userData[i].name.c_str());
                            wrap::Close(client[usrInRoom[i]].fd);
                            client[usrInRoom[i]].fd = -1;
                            currentNumberUser--;
                        }
                         else if (nSnd == userData[usrInRoom[i]].sndBuf.length())
                        {
                            if (client[usrInRoom[i]].events & POLLOUT)
                                client[usrInRoom[i]].events = client[usrInRoom[i]].events ^ POLLOUT;
                            userData[usrInRoom[i]].sndBuf.clear();
                        }
                        else if (nSnd < userData[usrInRoom[i]].sndBuf.length())
                        {
                            client[usrInRoom[i]].events = client[usrInRoom[i]].events | POLLOUT;
                            userData[usrInRoom[i]].sndBuf.erase(nSnd);
                        }

                        break;
                    }
            }

            room->ErasePublicMsg(itPublicMsg);
            itPublicMsg++;
        }

        if (!room->CheckEmptyMsg())
        {
            chat->RemoveRoomWaitHandler(itWaitRoom);
            room->SetWaitHandler(false);
            std::cout << "удаление комнаты из списка комнат требующих обработку, номер комнаты: " << *itWaitRoom << std::endl;
            std::cout.flush();
        }

        itWaitRoom++;

    }

    for (int i=2; i<netData->maxUser+2; i++)
    {
        if (client[i].fd != -1 && client[i].revents & POLLOUT)
        {

            nSnd = write(client[i].fd, userData[i].sndBuf.c_str(), userData[i].sndBuf.length());

            if (nSnd == -1 && errno == EPIPE)
            {
                chat->RemoveUsr(userData[usrInRoom[i]].posInRoom, userData[usrInRoom[i]].numberRoom, userData[i].name.c_str());
                wrap::Close(client[usrInRoom[i]].fd);
                client[usrInRoom[i]].fd = -1;
                currentNumberUser--;
            }
            else if (nSnd == userData[i].sndBuf.length())
            {
                client[usrInRoom[i]].events = client[usrInRoom[i]].events ^ POLLOUT;
                userData[i].sndBuf.clear();
            }
            else if (nSnd < userData[i].sndBuf.length())\
                userData[i].sndBuf.erase(nSnd);\

        }
    }
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
                sprintf(buff, "%d", (netData->numberThread));         // количество потоков в данный момент на appcontroller
                bodyMsg.append(buff);
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

                event.CreateMessage(bodyMsg.c_str(), app::NewAppModule, app::UI);

                appMsg.AddMessage(event, err);

                bodyMsg.erase();
                sprintf(buff, "%d", modID[1]);
                bodyMsg.append(buff);

                event.CreateMessage(bodyMsg.c_str(), app::NewAppModule, app::UI);

                appMsg.AddMessage(event, err);

                pthread_t threadCont;

                pthread_create(&threadCont, &attr, modules::AppController, appData);

                pthread_attr_destroy(&attr);            
            }
        }

        client[0].fd = -1;
        netData->createThread = true;
    }
}

ChatRoom::~ChatRoom()
{
    delete[] usrID;
}

void ChatRoom::ChatRoomInit(int valUsr)
{
    usrID = new unsigned int[valUsr];
    numUsr = valUsr;

    for (int i=0; i< valUsr; i++)
    {
        usrID[i] = -1;
    }

    availableSpace = valUsr;
}

void ChatRoom::AddPrivateMsg(PrivateMsg msg)
{
    queuePrivateMsg.push_back(msg);
}

void ChatRoom::AddPublicMsg(PublicMsg msg)
{
    queuePublicMsg.push_back(msg);
}

std::deque<PrivateMsg>::iterator ChatRoom::GetPrivateMsg(int& size)
{
    std::deque<PrivateMsg>::iterator it = queuePrivateMsg.begin();
    size = queuePrivateMsg.size();

    return it;
}

std::deque<PublicMsg>::iterator ChatRoom::GetPublicMsg(int& size)
{
    std::deque<PublicMsg>::iterator it = queuePublicMsg.begin();
    size = queuePublicMsg.size();

    return it;
}

void ChatRoom::ErasePrivateMsg(std::deque<PrivateMsg>::iterator itPrivateMsg)
{
    queuePrivateMsg.erase(itPrivateMsg);
}

void ChatRoom::ErasePublicMsg(std::deque<PublicMsg>::iterator itPublicMsg)
{
    queuePublicMsg.erase(itPublicMsg);
}

bool ChatRoom::CheckEmptyMsg()
{
    if (queuePrivateMsg.empty() && queuePublicMsg.empty())
        return false;
    else
        return true;
}

unsigned int* ChatRoom::GetUsrID()
{
    return usrID;
}

unsigned int ChatRoom::GetUsr(int pos)
{
    return usrID[pos];
}

bool ChatRoom::CheckAvailableSpace()
{
    if (availableSpace > 0)
        return true;
    else
        return false;
}

bool ChatRoom::CheckWaitHandler()
{
    return waitHandler;
}

void ChatRoom::SetWaitHandler(bool status)
{
    waitHandler = status;
}

unsigned int ChatRoom::AddUsr(int id)
{
    unsigned int i = 0;
    while (i < numUsr)
    {
        if (usrID[i] == -1)
        {
            usrID[i] = id;
            availableSpace--;
            break;
        }

        i++;
    }

    return i;
}

void ChatRoom::RemoveUsr(unsigned int pos)
{
    usrID[pos] = -1;
    availableSpace++;
}

Chat::Chat(int valRoom, int numUsrOnRoom)
{
    chatRoom = new ChatRoom[valRoom];
    numRoom = valRoom;

    for (int i=0; i<valRoom; i++)
        chatRoom[i].ChatRoomInit(numUsrOnRoom);
}

Chat::~Chat()
{
    delete[] chatRoom;
}

void Chat::AddUsr(int id, unsigned int& posInRoom, unsigned int& numberRoom, const char*name, int maxUser, UserData *usrData)
{
    unsigned int i = 0;
    while (i < numRoom)
    {
        if (chatRoom[i].CheckAvailableSpace())
        {
            std::cout << "пользователь добавляется в комнату: " << i << std::endl;
            posInRoom = chatRoom[i].AddUsr(id);
            PublicMsg pubMsg;
            pubMsg.idUser = posInRoom;
            pubMsg.msg.append(ADDNEWUSR);
            pubMsg.msg.append(",");
            pubMsg.msg.append(name);
            pubMsg.msg.append(".");
            char buff[10];
            sprintf(buff, "%d", posInRoom);
            pubMsg.msg.append(buff);
            chatRoom[i].AddPublicMsg(pubMsg);

            PrivateMsg prvtMsg;
            prvtMsg.idUserSnd = -1;
            prvtMsg.idUserRcv = posInRoom;
            prvtMsg.msg.append(ADDNEWUSR);

            unsigned int *usrID = chatRoom[i].GetUsrID();
            for (int i=0; i<maxUser; i++)
            {
                if (usrID[i] != -1 && posInRoom != i)
                {
                    prvtMsg.msg.append(",");
                    prvtMsg.msg.append(usrData[usrID[i]].name);
                    prvtMsg.msg.append(".");
                    sprintf(buff, "%d", i);
                    prvtMsg.msg.append(buff);
                }
            }
            chatRoom[i].AddPrivateMsg(prvtMsg);

            numberRoom = i;

            break;
        }

        i++;
    }

    if (!chatRoom[i].CheckWaitHandler())
    {
        std::cout << "добавление комнаты в очередь ожидающих обработки, номер комнаты: " << i << std::endl;
        std::cout.flush();
        roomWaitHandler.push_back(i);
        chatRoom[i].SetWaitHandler(true);
    }
}

void Chat::RemoveUsr(unsigned int posInRoom, unsigned int numberRoom, const char* name)
{
    chatRoom[numberRoom].RemoveUsr(posInRoom);

    PublicMsg msg;
    msg.idUser = posInRoom;
    msg.msg.append(REMOVEUSR);
    msg.msg.append(":");
    msg.msg.append(name);
    chatRoom[numberRoom].AddPublicMsg(msg);

    if (!chatRoom[numberRoom].CheckWaitHandler())
    {
        std::cout << "добавление комнаты в очередь ожидающих обработки, номер комнаты: " << numberRoom << std::endl;
        std::cout.flush();
        roomWaitHandler.push_back(numberRoom);
        chatRoom[numberRoom].SetWaitHandler(true);
    }
}

std::deque<int>::iterator Chat::GetIDRoomWaitHadler(int& size)
{
    std::deque<int>::iterator it = roomWaitHandler.begin();
    size = roomWaitHandler.size();

    return it;
}

ChatRoom* Chat::GetChatRoom(unsigned int num)
{
    return &chatRoom[num];
}

void Chat::SetRoomWaitHandler(unsigned int num)
{
    roomWaitHandler.push_back(num);
}

void Chat::RemoveRoomWaitHandler(std::deque<int>::iterator room)
{
    roomWaitHandler.erase(room);
}

bool InitNetModule(NetData& netData, app::AppMessage& dataMsg, app::AppSetting& dataSttng)
{
    app::SettingData sttngData;
    dataSttng.GetSetting(sttngData);
    netData.maxUser = sttngData.GetUserOnThread();
    netData.ratio = sttngData.GetRatioAppContAppNet();
    netData.userOnchatRoom = sttngData.GetUserOnChatRoom();
    netData.createThread = false;

    struct rlimit limit;
    getrlimit(RLIMIT_NOFILE, &limit);
    std::cout << limit.rlim_cur << ' ' << limit.rlim_max << std::endl;

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

    const char* textMsg = msg.GetBodyMsg();
    std::string bodyMsg = textMsg;

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
    ssize_t readSize;
    while((readSize = read(netData.pipe, buff, 4)) == -1)
    { std::cout << 1 << std::endl;}
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

    std::cout << "создание потока NetModule завершено успешно " << std::endl;
    std::cout.flush();

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
