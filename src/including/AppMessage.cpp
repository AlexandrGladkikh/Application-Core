#include "AppMessage.h"

#include <iostream>

namespace app {
////////////////////////////////////

Message::Message()
{
    bodyMessage = "";
    lengthMessage = 0;
    receiver = UnavailableName;
    sender = UnavailableName;
}

Message::~Message()
{

}

bool Message::CreateMessage(const char* textmsg, int rcv, int snd)
{
    if (sizeof(textmsg) == 0)
        bodyMessage = "";
    else
        bodyMessage = textmsg;

    lengthMessage = bodyMessage.size();

    receiver = rcv;
    sender = snd;

    return true;
}

void Message::SetRcv(int rcv)
{
    receiver = rcv;
}

void Message::SetSnd(int snd)
{
    sender = snd;
}

void Message::SetBodyMsg(std::string str)
{
    bodyMessage = str;
    lengthMessage = bodyMessage.size();
}

int Message::GetRcv()
{
    return receiver;
}

int Message::GetSnd()
{
    return sender;
}

std::string& Message::GetBodyMsg()
{
    return bodyMessage;
}

size_t Message::GetLength()
{
    return lengthMessage;
}

QueueMessage::QueueMessage()
{

}

QueueMessage::~QueueMessage()
{

}

void QueueMessage::AddMessage(Message msg, MsgError &qerror)
{
    Msg.push(msg);
    qerror = ErrorNot;
}

void QueueMessage::GetMessage(Message &msg, MsgError &qerror)
{
    msg = Msg.front();
    Msg.pop();

    qerror = ErrorNot;
}

void QueueMessage::EraseMsg()
{
    while (!Msg.empty())
    {
      Msg.pop();
    }
}

int QueueMessage::GetCountMsg()
{
    return Msg.size();
}

AppMessage::AppMessage()
{
    appClose = false;
}

AppMessage::~AppMessage()
{
    delete[] queueMessage;
    delete[] mutex;
    delete[] condition;
    delete[] arrModules;

    pthread_condattr_destroy(&attrCond);
    pthread_mutexattr_destroy(&attrMutex);

    for(int i=0; i<=maxModules; i++)
        pthread_mutex_destroy(&mutex[i]);

    for(int i=0; i<=maxModules; i++)
        pthread_cond_destroy(&condition[i]);
}

void AppMessage::SetThreadParam(int valueMaxModules)
{
    queueMessage = new QueueMessage[valueMaxModules];
    mutex = new pthread_mutex_t[valueMaxModules];
    condition = new pthread_cond_t[valueMaxModules];
    arrModules = new int[valueMaxModules];
    maxModules = valueMaxModules;
    statusmodule.SetMaxModule(valueMaxModules);

    arrModules[0] = 1;
    arrModules[1] = 1;
    arrModules[2] = 1;
    arrModules[3] = 1;
    for(int i=4; i<valueMaxModules; i++)
        arrModules[i] = -1;

    pthread_condattr_setpshared(&attrCond, PTHREAD_PROCESS_PRIVATE);

    pthread_condattr_init(&attrCond);

    for(int i=0; i<valueMaxModules; i++)
        pthread_cond_init(&condition[i], &attrCond);

    pthread_mutexattr_setpshared(&attrMutex, PTHREAD_PROCESS_PRIVATE);

    pthread_mutexattr_setprotocol(&attrMutex, PTHREAD_PRIO_INHERIT);

    pthread_mutexattr_init(&attrMutex);

    for(int i=0; i<valueMaxModules; i++)
        pthread_mutex_init(&mutex[i], &attrMutex);

    pthread_mutex_init(&mutexArrModules, &attrMutex);

    countModules = 4;
}

int AppMessage::AddNewModule()
{
    pthread_mutex_lock(&mutexArrModules);
    if (countModules != maxModules && appClose != true)
    {
        for(int i=0; i< maxModules; i++)
        if (arrModules[i] == -1)
        {
            arrModules[i] = 1;
            countModules++;
            return i;
        }
    }
    else
        return -1;
    pthread_mutex_unlock(&mutexArrModules);
}

bool AppMessage::DeleteModule(int id)
{
    pthread_mutex_lock(&mutexArrModules);
    if (appClose != true)
    {
        arrModules[id] = -1;
        countModules--;
        queueMessage[id].EraseMsg();
        return true;
    }
    else
        return false;
    pthread_mutex_unlock(&mutexArrModules);
}

int AppMessage::GetCountModules()
{
    return countModules;
}

void AppMessage::AddMessage(Message msg,  MsgError &qerror)
{
    if( ModuleNameFirst <= msg.GetRcv() && msg.GetRcv() < maxModules)
    {
        pthread_mutex_lock(&mutex[msg.GetRcv()]);

        queueMessage[msg.GetRcv()].AddMessage(msg, qerror);

        if (statusmodule.GetStatus(msg.GetRcv()) == StatusWait && qerror == ErrorNot)
            pthread_cond_signal(&condition[msg.GetRcv()]);

        pthread_mutex_unlock(&mutex[msg.GetRcv()]);
    }
    else
        qerror = ErrorNameModule;
}

void AppMessage::GetMessage(Message &msg, MsgError &qerror)
{

    if( ModuleNameFirst <= msg.GetRcv() && msg.GetRcv() < maxModules)
    {
        pthread_mutex_lock(&mutex[msg.GetRcv()]);

        if (queueMessage[msg.GetRcv()].GetCountMsg() == 0)
        {
            statusmodule.SetStatus(msg.GetRcv(), StatusWait);
            pthread_cond_wait(&condition[msg.GetRcv()], &mutex[msg.GetRcv()]);
            statusmodule.SetStatus(msg.GetRcv(), StatusWorks);
        }
        else if (statusmodule.GetStatus(msg.GetRcv()) == StatusWait)
        {
            statusmodule.SetStatus(msg.GetRcv(), StatusWorks);
        }

        queueMessage[msg.GetRcv()].GetMessage(msg, qerror);
        //std::cout << msg.GetBodyMsg().c_str() << msg.GetRcv() << msg.GetSnd() << std::endl;
        //std::cout.flush();

        pthread_mutex_unlock(&mutex[msg.GetRcv()]);
    }
    else
        qerror = ErrorNameModule;
}

int AppMessage::AddMessageAllModules(Message msg, MsgError &qerror)
{
    pthread_mutex_lock(&mutexArrModules);
        appClose = true;
    pthread_mutex_unlock(&mutexArrModules);

    for (int i = ModuleNameFirst; i < maxModules; i++)
    {
        if (msg.GetSnd()!=i && arrModules[i]!=-1)
        {
            pthread_mutex_lock(&mutex[i]);

            msg.SetRcv(i);
            queueMessage[i].AddMessage(msg, qerror);

            if (statusmodule.GetStatus(i) == StatusWait && qerror == ErrorNot)
                pthread_cond_signal(&condition[i]);

            pthread_mutex_unlock(&mutex[i]);
        }
    }

    return countModules;
}

////////////////////////////////////
}





























