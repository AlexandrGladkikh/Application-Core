#include "AppController.h"
#include "../including/AppData.h"
#include "../Wrap/Log.h"
#include "../../appcontroller/Appcontroller/appcont.h"
#include <stdlib.h>
#include <stdio.h>

namespace modules {
////////////////////////////////////

void* AppController(void *appData)
{
    app::AppData *data = (app::AppData*)appData;
    app::AppMessage &dataMsg = data->GetMsg();
    HandlerAppController handler(data);

    int selfID = AppControllerInit(&handler, &dataMsg);

    app::Message msg;
    msg.SetRcv(selfID);
    app::MsgError err;

    bool continuation = true;

    do
    {
        msg.SetRcv(selfID);
        dataMsg.GetMessage(msg, err);

        if (err != app::ErrorNot)
        {
            wrap::Log("Error get message\n", LOGAPPCONTROLLER);
            continue;
        }
        else
        {
            continuation = handler.handler(msg);
        }

    } while(continuation);
}

HandlerAppController::HandlerAppController(app::AppData *appData) : dataMsg(appData->GetMsg())
{
    data = appData;
    setting = data->GetSttng();

    selfID = app::appController;
}

bool HandlerAppController::handler(app::Message &event)
{
    std::string &bodyMsg = event.GetBodyMsg();
    std::string eventStr;
    size_t start;
    size_t end;

    start = bodyMsg.find(EVENTSTART);
    end = bodyMsg.find(EVENTEND);

    if (start == std::string::npos || end == std::string::npos)
    {

        if(!bodyMsg.compare(QUIT))
        {
            if (dataMsg.DeleteModule(selfID))
            {
                return false;
            }
            else
            {
                event.SetBodyMsg("close appcontroller\n");
                event.SetRcv(app::controller);
                event.SetSnd(app::appController);

                app::MsgError err;
                dataMsg.AddMessage(event, err);
            }

            return false;
        }
        else
        {
            wrap::Log("Error parse message\n", LOGAPPCONTROLLER);
        }
        return true;
    }
    else
    {
        eventStr = bodyMsg.substr(start+7, end);
    }

    if (AppCont(event))
    {
        app::MsgError err;
        dataMsg.AddMessage(event, err);
        return true;
    }

    if(!eventStr.compare(CREATENEWAPPCONTROLLER))
    {
        int newID = dataMsg.AddNewModule();
        int newNetID = atoi(bodyMsg.substr((bodyMsg.find(DATASTART)+6), bodyMsg.find(DATAEND)).c_str());

        pthread_attr_t attr;

        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

        pthread_attr_setschedpolicy(&attr, SCHED_RR);

        pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);

        pthread_attr_init(&attr);

        pthread_t thread;

        int rez = pthread_create(&thread, &attr, modules::AppController, data);
        if (rez != 0)
        {
            wrap::Log("Error create new appcontroller\n", LOGAPPCONTROLLER);
            exit(1);
        }

        std::string bodyMsg;
        bodyMsg.append(EVENTSTART);
        bodyMsg.append(SETSELFID);
        bodyMsg.append(EVENTEND);
        bodyMsg.append(DATASTART);
        char buf[10];
        sprintf(buf, "%d", newID);
        bodyMsg.append(buf);
        bodyMsg.append(DATAEND);

        event.CreateMessage(bodyMsg.c_str(), app::NewModule, selfID);
        app::MsgError err;

        dataMsg.AddMessage(event, err);

        if (err != app::ErrorNot)
        {
            wrap::Log("Error add message\n", LOGAPPCONTROLLER);
        }

        bodyMsg.erase();
        bodyMsg.append(EVENTSTART);
        bodyMsg.append(SETNETID);
        bodyMsg.append(EVENTEND);
        bodyMsg.append(DATASTART);
        sprintf(buf, "%d", newNetID);
        bodyMsg.append(buf);
        bodyMsg.append(DATAEND);

        event.CreateMessage(bodyMsg.c_str(), app::NewModule, selfID);

        dataMsg.AddMessage(event, err);

        if (err != app::ErrorNot)
        {
            wrap::Log("Error add message\n", LOGAPPCONTROLLER);
        }

        return true;
    }

    return true;
}

inline void HandlerAppController::SetSelfID(int id)
{
    selfID = id;
}

int AppControllerInit(HandlerAppController* handler, app::AppMessage *dataMsg)
{
    app::Message msg;
    msg.SetRcv(app::NewModule);
    app::MsgError err;

    dataMsg->GetMessage(msg, err);

    std::string &bodyMsg = msg.GetBodyMsg();

    int newID = atoi(bodyMsg.substr((bodyMsg.find(DATASTART)+6), bodyMsg.find(DATAEND)).c_str());
    handler->SetSelfID(newID);

    dataMsg->GetMessage(msg, err);

    bodyMsg = msg.GetBodyMsg();

    int newNetID = atoi(bodyMsg.substr((bodyMsg.find(DATASTART)+6), bodyMsg.find(DATAEND)).c_str());

    bodyMsg.erase();
    bodyMsg.append(EVENTSTART);
    bodyMsg.append(START);
    bodyMsg.append(EVENTEND);
    bodyMsg.append(DATASTART);
    char buf[10];
    sprintf(buf, "%d", newID);
    bodyMsg.append(buf);
    bodyMsg.append(DATAEND);

    msg.CreateMessage(bodyMsg.c_str(), newNetID, newID);

    dataMsg->AddMessage(msg, err);

    return newID;
}

////////////////////////////////////
}





































