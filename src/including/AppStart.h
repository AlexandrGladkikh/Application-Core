#ifndef APPSTART_H
#define APPSTART_H

#pragma once

#include "../including/AppData.h"
#include "AppDefined.h"

namespace app {
////////////////////////////////////

struct ThreadData
{
    pthread_attr_t attr;

    pthread_t threads[ModuleNameEnd];

    ThreadData();
    ~ThreadData();

    bool GetThread(int numThread, pthread_t& thread);
    void GetAttr(pthread_attr_t& attrObj);
};

bool StartX(AppData *appData);

////////////////////////////////////
}

#endif // APPSTART_H
