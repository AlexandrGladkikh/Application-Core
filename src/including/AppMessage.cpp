#include "AppMessage.h"
#include "../Wrap/WrapNet.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

namespace app {
////////////////////////////////////

Message::Message()
{
    //bodyMessage = "";
    lengthMessage = 0;
    receiver = UnavailableName;
    sender = UnavailableName;
}

Message::~Message()
{

}

bool Message::CreateMessage(const char* textmsg, int rcv, int snd)
{
    if (!strlen(textmsg) == 0)
     //   bodyMessage = "";
    //else
    {
        memcpy(bodyMessage, textmsg, strlen(textmsg)+1);
    }

    lengthMessage = strlen(bodyMessage);

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
    memcpy(bodyMessage, str.c_str(), str.length());
    lengthMessage = strlen(bodyMessage);
}

int Message::GetRcv()
{
    return receiver;
}

int Message::GetSnd()
{
    return sender;
}

const char* Message::GetBodyMsg()
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
    delete[] sockNetModule;

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
    sockNetModule = new int[valueMaxModules];

    maxModules = valueMaxModules;
    statusmodule.SetMaxModule(valueMaxModules);

    for(int i=0; i < BASICMODULES; i++)
        arrModules[i] = 1;

    for(int i=BASICMODULES; i<valueMaxModules; i++)
        arrModules[i] = -1;

    for(int i=0; i<valueMaxModules; i++)
        sockNetModule[i] = -1;

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

    countModules = BASICMODULES;
}

void AppMessage::AddNewModule(int *modID)
{
    pthread_mutex_lock(&mutexArrModules);
    if (countModules < maxModules && appClose != true)
    {
        for(int i=0; i < maxModules; i++)
        if (arrModules[i] == -1)
        {
            arrModules[i] = 1;
            countModules++;
            *modID = i;
            break;
        }
    }
    else
        *modID = -1;
    pthread_mutex_unlock(&mutexArrModules);
}

void AppMessage::AddNewModule(int *sockPipe, int *id)
{
    pthread_mutex_lock(&mutexArrModules);
    if (countModules < maxModules && appClose != true)
    {
        for(int i=0; i < maxModules; i++)
        if (arrModules[i] == -1)
        {
            arrModules[i] = 1;
            countModules++;
            int sock[2];
            socketpair(AF_UNIX, SOCK_STREAM, 0, sock);
            sockNetModule[i] = sock[0];
            *sockPipe = sock[1];
            *id =  i;
            break;
        }
    }
    else
    {
        sockPipe[1] = -1;
    }
    pthread_mutex_unlock(&mutexArrModules);
}

void AppMessage::DeleteModule(int id)
{
    pthread_mutex_lock(&mutexArrModules);
    if (appClose != true)
    {
        arrModules[id] = -1;
        countModules--;
        queueMessage[id].EraseMsg();
        if (sockNetModule[id]!=-1)
        {
            wrap::Close(sockNetModule[id]);
            sockNetModule[id] = -1;
        }
        id = 1;
    }
    else
        id = 0;
    pthread_mutex_unlock(&mutexArrModules);
}

int AppMessage::GetCountModules()
{
    return countModules;
}

int AppMessage::GetMaxModules()
{
    return maxModules;
}

void AppMessage::AddMessage(Message msg,  MsgError &qerror)
{
    if( ModuleNameFirst <= msg.GetRcv() && msg.GetRcv() < maxModules)
    {
        pthread_mutex_lock(&mutex[msg.GetRcv()]);

        queueMessage[msg.GetRcv()].AddMessage(msg, qerror);

        if (sockNetModule[msg.GetRcv()] != -1)
        {
            int nRcv;
            while (1)
            {
                nRcv = write(sockNetModule[msg.GetRcv()], "read", 4);
                if ((nRcv != -1) && (nRcv != 0))
                    break;
            }
        }
        else if (statusmodule.GetStatus(msg.GetRcv()) == StatusWait && qerror == ErrorNot)
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
    int nRcv;

    for (int i = ModuleNameFirst; i < maxModules; i++)
    {
        if (msg.GetSnd()!=i && arrModules[i]!=-1)
        {
            pthread_mutex_lock(&mutex[i]);

            msg.SetRcv(i);
            queueMessage[i].AddMessage(msg, qerror);

            if (sockNetModule[i] != -1)
                while (1)
                {
                    nRcv = write(sockNetModule[msg.GetRcv()], "read", 4);
                    if ((nRcv != -1) && (nRcv != 0))
                        break;
                }
            else if (statusmodule.GetStatus(i) == StatusWait && qerror == ErrorNot)
            {
                pthread_cond_signal(&condition[i]);
            }

            pthread_mutex_unlock(&mutex[i]);
        }
    }

    return countModules;
}

////////////////////////////////////
}





























