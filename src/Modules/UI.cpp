#include "UI.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "errno.h"
#include <string.h>
#include <sys/select.h>
#include "../Wrap/WrapNet.h"

namespace modules
{
////////////////////////////////////

bool UI(app::AppData *appData)
{
    app::AppMessage &data = appData->GetMsg();

    EventData eventData;

    UIHandler handler(appData);

    bool continuation = true;
    bool success = true;

    EventDataInit(&eventData, &data);

    do
    {

    std::cout << '>';
    std::cout.flush();

    WaitEvent(&eventData);

    continuation = handler.Handler(&eventData);

    } while(continuation);

    success = AppExit(&eventData, &data);

    return success;
}

fd_set* EventData::GetFdSet()
{
    return &rset;
}

int EventData::GetMaxFdp()
{
    return maxfdp;
}

void EventData::SetMaxFdp(int maxFdp)
{
    maxfdp = maxFdp;
}

char* EventData::GetEvent()
{
    return event;
}

int* EventData::GetSocketPipe()
{
    return socketsPipe;
}

int EventData::GetSocketPipe1()
{
    return socketsPipe[0];
}

int EventData::GetSocketPipe2()
{
    return socketsPipe[1];
}

size_t EventData::GetSizeBuf()
{
    return sizeBuf;
}

UIBehavior::UIBehavior()
{
    for(int i=FIRSTEVENTS; i<EVENTEND; i++)
    {
        condition[i] = UnavailableEvent;
    }
}

void UIBehavior::SetBehavior(int event)
{
    if (FIRSTEVENTS<=event && event<EVENTEND)
    {
        condition[event] = (Event)event;
    }
}

bool UIBehavior::TestSetEvent(int event)
{
    if (FIRSTEVENTS<=event && event<EVENTEND)
    {
        return (condition[event]==event);
    }
    return false;
}

int UIHandlerData::GetHandlerEvent()
{
    return event;
}

char* UIHandlerData::GetHandlerParam()
{
    return param;
}

void UIHandlerData::SetHandlerEvent(int evnt)
{
    event = evnt;
}

void UIHandlerData::SetHandlerParam(char* parm)
{
    param = parm;
}

UIHandler::UIHandler(app::AppData *appData) : dataMsg(appData->GetMsg()), setting(appData->GetSttng())
{
    for(int i=FIRSTEVENTS; i<EVENTEND; i++)
    {
        behavior[i].SetBehavior(i);
    }
}

UIHandler::~UIHandler()
{

}

void UIHandler::Parser(char* event, UIHandlerData* handlerData)
{
    char* param;

    if (!strcmp(event, EXIT))
    {
        handlerData->SetHandlerEvent(APPEXIT);
        return;
    }
    else if (!strcmp(event, LIST))
    {
        handlerData->SetHandlerEvent(LISTSETTING);
        return;
    }
    else if (strstr(event, SET) != NULL)
    {        
        if ((param = strstr(event, "-")) == NULL)
        {
            handlerData->SetHandlerEvent(-1);
            return;
        }

        handlerData->SetHandlerParam(param);

        handlerData->SetHandlerEvent(SETSETTING);

        return;
    }
    else if (strstr(event, SHOW) != NULL)
    {
        if ((param = strstr(event, "-")) == NULL)
        {
            handlerData->SetHandlerEvent(-1);
            return;
        }

        handlerData->SetHandlerParam(param);

        handlerData->SetHandlerEvent(SHOWDATA);

        return;
    }

    handlerData->SetHandlerEvent(-1);
}

bool UIHandler::EventHandler(int behaviorEvent, const char* param)
{
        if (behavior[behaviorEvent].TestSetEvent(APPEXIT))
        {
            return false;
        }
        else if (behavior[behaviorEvent].TestSetEvent(SETSETTING))
        {
            app::SettingData sttng;
            setting.GetSetting(sttng);

            if (!strcmp(param, "-IPv4"))
            {
                std::cout << "IPv4" << std::endl;
                sttng.SetNetProtocol(app::NetIPv4);
            }
            else if (!strcmp(param, "-IPv6"))
            {
                std::cout << "IPv6" << std::endl;
                sttng.SetNetProtocol(app::NetIPv6);
            }
            else if (!strcmp(param, "-UnavailableProtocol"))
            {
                std::cout << "UnavailableProtocol" << std::endl;
                sttng.SetNetProtocol(app::UnavailableProtocol);
            }
            else if (!strcmp(param, "-NetNoLinked"))
            {
                std::cout << "NetNoLinked" << std::endl;
                sttng.SetNetLink(app::NetNoLinked);
            }
            else if (!strcmp(param, "-NetAllLinked"))
            {
                std::cout << "NetAllLinked" << std::endl;
                sttng.SetNetLink(app::NetAllLinked);
            }
            else if (!strcmp(param, "-NetSelectiveLink"))
            {
                std::cout << "NetSelectiveLink" << std::endl;
                sttng.SetNetLink(app::NetSelectiveLink);
            }
            else if (!strcmp(param, "-UnavailableLinked"))
            {
                std::cout << "UnavailableLinked" << std::endl;
                sttng.SetNetLink(app::UnavailableLinked);
            }
            else if (!strcmp(param, "-NoAdd"))
            {
                std::cout << "NoAdd" << std::endl;
                sttng.SetDataAdd(app::NoAdd);
            }
            else if (!strcmp(param, "-AllAdd"))
            {
                std::cout << "AllAdd" << std::endl;
                sttng.SetDataAdd(app::AllAdd);
            }
            else if (!strcmp(param, "-UnavailableAdd"))
            {
                std::cout << "UnavailableAdd" << std::endl;
                sttng.SetDataAdd(app::UnavailableAdd);
            }
            else if (!strcmp(param, "-NoRead"))
            {
                std::cout << "NoRead" << std::endl;
                sttng.SetDataRead(app::NoRead);
            }
            else if (!strcmp(param, "-AllRead"))
            {
                std::cout << "AllRead" << std::endl;
                sttng.SetDataRead(app::AllRead);
            }
            else if (!strcmp(param, "-UnavailableRead"))
            {
                std::cout << "UnavailableRead" << std::endl;
                sttng.SetDataRead(app::UnavailableRead);
            }

            else if (!strcmp(param, "-NoWrite"))
            {
                std::cout << "NoWrite" << std::endl;
                sttng.SetDataWrite(app::NoWrite);
            }
            else if (!strcmp(param, "-AllWrite"))
            {
                std::cout << "AllWrite" << std::endl;
                sttng.SetDataWrite(app::AllWrite);
            }
            else if (!strcmp(param, "-UnavailableWrite"))
            {
                std::cout << "UnavailableWrite" << std::endl;
                sttng.SetDataWrite(app::UnavailableWrite);
            }

            else if (!strcmp(param, "-NoDel"))
            {
                std::cout << "NoDel" << std::endl;
                sttng.SetDataDel(app::NoDel);
            }
            else if (!strcmp(param, "-AllDel"))
            {
                std::cout << "AllDel" << std::endl;
                sttng.SetDataDel(app::AllDel);
            }
            else if (!strcmp(param, "-UnavailableDel"))
            {
                std::cout << "UnavailableDel" << std::endl;
                sttng.SetDataDel(app::UnavailableDel);
            }
            else if (!strcmp(param, "-setMaxThread"))
            {
                int value;
                std::cout << "Введите максимальное количество модулей(>7)" << std::endl;
                do
                {
                std::cin >> value;
                } while(value < 7);
                sttng.SetMaxModules(value);
            }
            else if (!strcmp(param, "-setServ"))
            {
                std::string value;
                std::cout << "Введите порт сервера:" << std::endl;
                std::cin >> value;
                sttng.SetServ(value.c_str());
            }
            else if (!strcmp(param, "-setHost"))
            {
                std::string value;
                std::cout << "Введите адрес сервера:" << std::endl;
                std::cin >> value;
                sttng.SetHost(value.c_str());
            }
            else
            {
                std::cout << ">incorrect param" << std::endl;
            }

            setting.SetSetting(sttng);
            return setting.SaveSetting();
        }
        else if (behavior[behaviorEvent].TestSetEvent(LISTSETTING))
        {
            app::SettingData sttng;
            setting.GetSetting(sttng);
            sttng.ListSetting();
            return true;
        }
        else if (behavior[behaviorEvent].TestSetEvent(SHOWDATA))
        {
            std::cout << param+1 << std::endl;
            return true;
        }

    return true;
}

bool UIHandler::Handler(EventData* eventData)
{
    UIHandlerData handlerData;

    Parser(eventData->GetEvent(), &handlerData);

    if (handlerData.GetHandlerEvent() == -1)
    {
        std::cout << ">incorrect command"  << std::endl;
        return true;
    }

    if (EventHandler(handlerData.GetHandlerEvent(), handlerData.GetHandlerParam()))
    {
        return true;
    }
    else
    {
        std::cout << ">close app..."  << std::endl;
        return false;
    }
}

void EventDataInit(EventData* eventData, app::AppMessage* msg)
{
    app::Message sendMsg;
    app::MsgError sendErr;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, eventData->GetSocketPipe()))
    {
        std::cout << "Error create pipe" << std::endl;
        exit(1);
    }

    size_t sizeRcvBuf = 52000;

    setsockopt(eventData->GetSocketPipe1(), SOL_SOCKET, SO_RCVBUF, &sizeRcvBuf, sizeof(int));

    eventData->SetMaxFdp(std::max(fileno(stdin), eventData->GetSocketPipe1())+1);

    snprintf(eventData->GetEvent(), eventData->GetSizeBuf(), "%d", eventData->GetSocketPipe2());

    sendMsg.CreateMessage(eventData->GetEvent(), app::controller, app::UI);

    msg->AddMessage(sendMsg, sendErr);

    if (sendErr != app::ErrorNot)
    {
        std::cout << "Error init pipe UI to controller" << std::endl;
        exit(1);
    }
}

void WaitEvent(EventData* eventData)
{
    int nReady;
    int nRcv;

    FD_ZERO(eventData->GetFdSet());

    FD_SET(fileno(stdin), eventData->GetFdSet());
    FD_SET(eventData->GetSocketPipe1(), eventData->GetFdSet());
    while ((nReady = wrap::Select(eventData->GetMaxFdp(), eventData->GetFdSet(), NULL, NULL, NULL)))
    {
        if (FD_ISSET(fileno(stdin), eventData->GetFdSet()))
        {
            std::cin >> eventData->GetEvent();
            break;
        }

        if (FD_ISSET(eventData->GetSocketPipe1(), eventData->GetFdSet()))
        {
            nRcv = read(eventData->GetSocketPipe1(), eventData->GetEvent(), eventData->GetSizeBuf());

            eventData->GetEvent()[nRcv] = '\0';
            break;
        }

        if (errno == EINTR)
        {
            FD_SET(fileno(stdin), eventData->GetFdSet());
            FD_SET(eventData->GetSocketPipe1(), eventData->GetFdSet());
            continue;
        }
    }
}

bool AppExit(EventData* eventData, app::AppMessage* data)
{
    app::Message msg;
    app::MsgError err;
    msg.CreateMessage(EXIT, app::controller, app::UI);
    data->AddMessage(msg, err);

    read(eventData->GetSocketPipe1(), eventData->GetEvent(), eventData->GetSizeBuf());

    return atoi(eventData->GetEvent());
}

////////////////////////////////////
}





































