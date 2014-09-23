#ifndef UI_H
#define UI_H

#pragma once

#include "../including/AppData.h"

namespace modules {
////////////////////////////////////

enum Event
{
FIRSTEVENTS = 0,
APPEXIT = 0,
SETSETTING = 1,
LISTSETTING = 2,
SHOWDATA = 3,
EVENTEND = 4,
UnavailableEvent = -1
};

#define EXIT "quit"
#define SET "setsetting"
#define LIST "settingl"
#define SHOW "show"

struct EventData
{
    static const size_t sizeBuf = 100;
    char event[sizeBuf];
    int maxfdp;
    fd_set rset;
    int socketsPipe[2];

    fd_set* GetFdSet();
    int GetMaxFdp();
    char* GetEvent();
    int* GetSocketPipe();
    int GetSocketPipe1();
    int GetSocketPipe2();
    size_t GetSizeBuf();

    void SetMaxFdp(int maxFdp);
};

struct UIBehavior
{
    Event condition[EVENTEND];

    UIBehavior();
    void SetBehavior(int event);
    bool TestSetEvent(int event);
};

struct UIHandlerData
{
    int event;
    char *param;

    int GetHandlerEvent();
    char* GetHandlerParam();

    void SetHandlerEvent(int evnt);
    void SetHandlerParam(char* parm);
};

class UIHandler
{
private:
    app::AppMessage &dataMsg;
    app::AppSetting &setting;

    UIBehavior behavior[EVENTEND];

    void Parser(char* event, UIHandlerData* handlerData);

    bool EventHandler(int behaviorEvent, const char* param);
public:
    UIHandler(app::AppData *appData);
    ~UIHandler();

    bool Handler(EventData* eventData);

};

bool UI(app::AppData *appData);

bool AppExit(EventData* eventData, app::AppMessage* data);

void EventDataInit(EventData* eventData, app::AppMessage* msg);

void WaitEvent(EventData* eventData);

////////////////////////////////////
}

#endif // UI_H






























