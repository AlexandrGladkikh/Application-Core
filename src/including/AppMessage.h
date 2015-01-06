#ifndef APPMESSAGE_H
#define APPMESSAGE_H

#pragma once

#include "AppStatus.h"
#include <string>
#include <string.h>
#include <queue>
#include <pthread.h>
#include "AppDefined.h"

namespace app {
////////////////////////////////////

    enum MsgError
    {
        ErrorNot,
        ErrorQueue,
        ErrorNameModule
    };

    struct Message
    {
        int receiver;
        int sender;

        char bodyMessage[1000];

        size_t lengthMessage;

        Message();
        ~Message();

        bool CreateMessage(const char* textmsg, int rcv, int snd);
        void SetRcv(int rcv);
        void SetSnd(int snd);
        void SetBodyMsg(std::string str);

        int GetRcv();
        int GetSnd();
        const char* GetBodyMsg();
        size_t GetLength();
    };

    class QueueMessage
    {
    private:
        std::queue<Message> Msg;
    public:
        QueueMessage();
        ~QueueMessage();

        void AddMessage(Message msg, MsgError &qerror);
        void GetMessage(Message &msg, MsgError &qerror);
        void EraseMsg();
        int GetCountMsg();
    };

    class AppMessage
    {
    private:
        ModuleState statusmodule;

        QueueMessage *queueMessage;

        pthread_mutexattr_t attrMutex;
        pthread_condattr_t attrCond;

        pthread_mutex_t *mutex;
        pthread_cond_t *condition;

        int *arrModules;

        pthread_mutex_t mutexArrModules;

        int *sockNetModule;

        int countModules;
        int maxModules;

        bool appClose;
    public:
        AppMessage();
        ~AppMessage();
        void SetThreadParam(int countThread);
        void AddNewModule(int *modID);
        void AddNewModule(int *sockPipe, int *id);
        void DeleteModule(int id);
        int GetCountModules();
        int GetMaxModules();

        void AddMessage(Message msg, MsgError &qerror);
        void GetMessage(Message &msg, MsgError &qerror);
        int AddMessageAllModules(Message msg, MsgError &qerror);
    };
}

////////////////////////////////////
#endif // APPMESSAGE_H
