#ifndef CONTROLLER_H
#define CONTROLLER_H

#pragma once

#include "../including/AppData.h"
#include <string>

namespace modules {
////////////////////////////////////

#define LOGCONTROLLER "logcontroller.txt"

enum Event
{
FIRSTEVENTS = 0,
SHOWDATA = 0,
LOGDATA = 1,
APPEXIT = 2,
EVENTEND = 3,
UnavailableEvent = -1
};

#define SHOW "show"
#define LOG "log"
#define EXIT "quit"

struct UILink
{
    int sockUI;
    size_t sizeWindow;

    int GetLink();
    void SetLink(int link);
};

struct ControllerBehavior
{
    int condition[EVENTEND];

    ControllerBehavior();
    void SetBehavior(int event);
    bool TestSetEvent(int event);
};

struct ControllerHandlerData
{
    int event;
    std::string param;

    int GetHandlerEvent();
    std::string GetHandlerParam();

    void SetHandlerEvent(int evnt);
    void SetHandlerParam(std::string parm);
};

class ControllerHandler
{
private:
    app::AppMessage *dataMsg;
    UILink *linkUI;

    ControllerBehavior behavior[EVENTEND];

    void Parser(std::string event, ControllerHandlerData* handlerData);

    bool EventHandler(int behaviorEvent, std::string param);
public:
    ControllerHandler(app::AppMessage *appMsg, UILink *uiLink);
    ~ControllerHandler();

    bool Handler(app::Message* msg);

};

void* Controller(void* appData);

void InitUILink(app::AppMessage* msg, UILink* link);

////////////////////////////////////
}

#endif // CONTROLLER_H
