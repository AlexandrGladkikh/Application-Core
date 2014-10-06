#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#pragma once

#include "../including/AppData.h"

namespace modules {
////////////////////////////////////

#define LOGAPPCONTROLLER "logappcontroller.txt"

#define QUIT                    "quit"
#define CREATENEWAPPCONTROLLER  "create"
#define SETSELFID               "selfid"
#define SETNETID                "netid"
#define START                   "start"


#define DATASTART               "<data>"
#define DATAEND                 "</data>"
#define EVENTSTART              "<event>"
#define EVENTEND                "</event>"

void* AppController(void *appData);

class HandlerAppController
{
private:
    app::AppData *data;
    app::AppMessage &dataMsg;

    int selfID;
public:
    HandlerAppController(app::AppData *appData);
    inline void SetSelfID(int id);

    bool handler(app::Message &event);
};

int AppControllerInit(HandlerAppController* handler, app::AppMessage *dataMsg);

////////////////////////////////////
}

#endif // APPCONTROLLER_H
