#include "AppStart.h"
#include "../Modules/Controller.h"
#include "../Modules/ModuleData.h"
#include "../Modules/AppController.h"
#include "../Modules/NetModule.h"
#include "../Wrap/Log.h"
#include "../Wrap/WrapNet.h"

#include <iostream>

namespace app {
////////////////////////////////////

ThreadData::ThreadData()
{
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    pthread_attr_setschedpolicy(&attr, SCHED_RR);

    pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);

    pthread_attr_init(&attr);
}

bool ThreadData::GetThread(int numThread, pthread_t& thread)
{
    if (ModuleNameFirst <= numThread && numThread < ModuleNameEnd)
    {
        thread = threads[numThread];
        return true;
    }

    return false;
}

void ThreadData::GetAttr(pthread_attr_t &attrObj)
{
    attrObj = attr;
}

bool StartX(AppData *appData)
{
    app::ThreadData threadData;

    pthread_t thread;
    pthread_attr_t attrObj;
    threadData.GetAttr(attrObj);

    threadData.GetThread(controller, thread);

    int rez = pthread_create(&thread, &attrObj, modules::Controller, appData);
    if (rez != 0)
        return false;

    threadData.GetThread(moduleData, thread);

    rez = pthread_create(&thread, &attrObj, modules::ModuleData, appData);
    if (rez != 0)
        return false;

    ////////////////////////////////////
    AppMessage &dataMsg = appData->GetMsg();
    SettingData dataSttng;
    appData->GetSttng().GetSetting(dataSttng);
    int modID[2];
    dataMsg.AddNewModule(&modID[0], &modID[1]);

    socklen_t len;
    int sockfd;
    if ((sockfd = wrap::CreateSocket(dataSttng.GetHost(), dataSttng.GetServ(), &len)) == 0)
        return false;

    std::string bodyMsg;
    app::Message event;
    app::MsgError err;

    bodyMsg.append(EVENTSTART);
    bodyMsg.append(SETSELFID);
    bodyMsg.append(EVENTEND);
    bodyMsg.append(DATASTART);
    char buff[10];
    sprintf(buff, "%d", modID[0]);
    bodyMsg.append(buff);
    bodyMsg.append("//");
    sprintf(buff, "%d", modID[1]);
    bodyMsg.append(buff);
    bodyMsg.append("//");
    sprintf(buff, "%d", sockfd);
    bodyMsg.append(buff);
    bodyMsg.append(DATAEND);
    event.CreateMessage(bodyMsg.c_str(), app::NewNetModule, app::UI);
    dataMsg.AddMessage(event, err);

    if (err != app::ErrorNot)
    {
        wrap::Log("Error add message\n", APPSTART);
        return false;
    }

    threadData.GetThread(netModule, thread);

    rez = pthread_create(&thread, &attrObj, modules::NetModule, appData);
    if (rez != 0)
        return false;

    ////////////////////////////////////
    int id;
    dataMsg.AddNewModule(&id);

    bodyMsg.erase();
    bodyMsg.append(EVENTSTART);
    bodyMsg.append(SETSELFID);
    bodyMsg.append(EVENTEND);
    bodyMsg.append(DATASTART);
    sprintf(buff, "%d", id);
    bodyMsg.append(buff);
    bodyMsg.append(DATAEND);

    event.CreateMessage(bodyMsg.c_str(), app::NewAppModule, app::UI);

    dataMsg.AddMessage(event, err);

    if (err != app::ErrorNot)
    {
        wrap::Log("Error add message\n", APPSTART);
        return false;
    }

    bodyMsg.erase();
    bodyMsg.append(EVENTSTART);
    bodyMsg.append(SETNETID);
    bodyMsg.append(EVENTEND);
    bodyMsg.append(DATASTART);
    sprintf(buff, "%d", modID[1]);
    bodyMsg.append(buff);
    bodyMsg.append(DATAEND);

    event.CreateMessage(bodyMsg.c_str(), app::NewAppModule, app::UI);

    dataMsg.AddMessage(event, err);

    if (err != app::ErrorNot)
    {
        wrap::Log("Error add message\n", APPSTART);
        return false;
    }

    threadData.GetThread(appController, thread);

    rez = pthread_create(&thread, &attrObj, modules::AppController, appData);
    if (rez != 0)
        return false;
    ////////////////////////////////////

    return true;
}

////////////////////////////////////
}

































