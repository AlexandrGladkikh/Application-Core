#include "ModuleData.h"
#include "../including/AppData.h"
#include "../Wrap/Log.h"

namespace modules {
////////////////////////////////////

void* ModuleData(void *appData)
{
    app::AppData *data= (app::AppData *)appData;
    app::AppMessage &dataMsg = data->GetMsg();
    app::AppSetting &dataSetting = data->GetSttng();

    DataHandler handler(&dataSetting, &dataMsg);

    app::Message msg;
    msg.SetRcv(app::moduleData);
    app::MsgError err;

    bool continuation = true;

    do
    {        
        msg.SetRcv(app::moduleData);
        dataMsg.GetMessage(msg, err);

        if (err != app::ErrorNot)
        {
            wrap::Log("Error get message\n", LOGMODULEDATA);
            continue;
        }
        else
        {
            continuation = handler.Handler(msg);
        }

    } while(continuation);
}

DataHandler::DataHandler(app::AppSetting *dataSttng, app::AppMessage *dataMsg)
{
    dataSttng->GetSetting(setting);
    msg = dataMsg;
}
DataHandler::~DataHandler()
{

}

bool DataHandler::Handler(app::Message& event)
{
    std::string& bodyMsg = event.GetBodyMsg();
    std::string eventStr;
    std::string nameRcv;
    size_t posEventStart;
    size_t posEventEnd;
    size_t start;
    size_t end;

    start = bodyMsg.find(NAMERCVSTART);
    end = bodyMsg.find(NAMERCVEND);

    if ((posEventStart = bodyMsg.find(EVENTSTART)) == std::string::npos || (posEventEnd = bodyMsg.find(EVENTEND)) == std::string::npos)
    {
        ////////////////////////////////////////
        if (!bodyMsg.compare(QUIT))
        {
            event.SetBodyMsg("close moduleData");
            event.SetRcv(event.GetSnd());
            event.SetSnd(app::moduleData);

            app::MsgError err;
            msg->AddMessage(event, err);
            return false;
        }
        else
        {
            BadRequest(bodyMsg, event, "BADREQUEST");
        }
        return true;
    }
    else
    {
        eventStr = bodyMsg.substr((posEventStart+7), (posEventEnd - (posEventStart+7)));
    }
    ////////////////////////////////////////

    if (start == std::string::npos || end == std::string::npos)
    {
        BadRequest(bodyMsg, event, BADREQUEST);
        return true;
    }
    else
    {
        nameRcv = bodyMsg.substr((start+9), (end - (start+9)));
    }

    //////////////////////////////////////////////////////
    if (!eventStr.compare(ADDDATA))
    {
        if (setting.GetDataAdd() == app::AllAdd)
        {
            std::unordered_map<std::string , DataStorage>::iterator it = data.find(nameRcv);
            if (it != data.end())
            {
                bodyMsg.erase(start, (end+10) - start);

                if ((*it).second.ExecutionEvent(event, nameRcv) == false)
                {
                    BadRequest(bodyMsg, event, BADREQUEST);
                    return true;
                }

                if ((*it).second.TestWaitData())
                {
                    app::MsgError err;
                    msg->AddMessage(event, err);

                    if (err != app::ErrorNot)
                        wrap::Log(BADADDMSG, LOGMODULEDATA);
                    else
                        (*it).second.SetWaitData(false);
                }
                return true;
            }
            else
            {
                BadRequest(bodyMsg, event, NOTUSR);
                return true;
            }
        }
        else if(setting.GetDataAdd() == app::NoAdd)
        {
            BadRequest(bodyMsg, event, NOTADD);
            return true;
        }
    }
    ////////////////////////////////////////
    if (!eventStr.compare(READDATA))
    {
        if (setting.GetDataRead() == app::AllRead)
        {
            std::unordered_map<std::string , DataStorage>::iterator it = data.find(nameRcv);
            if (it != data.end())
            {
                bodyMsg.erase(start, ((end+10) - start));

                if ((*it).second.ExecutionEvent(event, nameRcv) == false)
                {
                    BadRequest(bodyMsg, event, BADREQUEST);
                    return true;
                }

                app::MsgError err;
                msg->AddMessage(event, err);

                if (err != app::ErrorNot)
                    wrap::Log(BADADDMSG, LOGMODULEDATA);

                return true;
            }
            else
            {
                BadRequest(bodyMsg, event, NOTUSR);
                return true;
            }
        }
        else if(setting.GetDataRead() == app::NoRead)
        {
            BadRequest(bodyMsg, event, NOTREAD);
            return true;
        }
    }
    ////////////////////////////////////////
    if (!eventStr.compare(WRITEDATA))
    {
        if (setting.GetDataWrite() == app::AllWrite)
        {
            std::unordered_map<std::string , DataStorage>::iterator it = data.find(nameRcv);
            if (it != data.end())
            {
                bodyMsg.erase(start, (end+10) - start);

                if ((*it).second.ExecutionEvent(event, nameRcv) == false)
                {
                    BadRequest(bodyMsg, event, BADREQUEST);
                    return true;
                }

                if ((*it).second.TestWaitData())
                {
                    app::MsgError err;
                    msg->AddMessage(event, err);

                    if (err != app::ErrorNot)
                        wrap::Log(BADADDMSG, LOGMODULEDATA);
                    else
                        (*it).second.SetWaitData(false);
                }

                return true;
            }
            else
            {
                BadRequest(bodyMsg, event, NOTUSR);
                return true;
            }
        }
        else if(setting.GetDataWrite() == app::NoWrite)
        {
            BadRequest(bodyMsg, event, NOTWRITE);
            return true;
        }
    }
    ////////////////////////////////////////
    if (!eventStr.compare(DELDATA))
    {
        if (setting.GetDataDel() == app::AllDel)
        {
            std::unordered_map<std::string , DataStorage>::iterator it = data.find(nameRcv);
            if (it != data.end())
            {
                bodyMsg.erase(start, (end+10) - start);

                if ((*it).second.ExecutionEvent(event, nameRcv) == false)
                {
                    BadRequest(bodyMsg, event, BADREQUEST);
                    return true;
                }

                return true;
            }
            else
            {
                BadRequest(bodyMsg, event, NOTUSR);
                return true;
            }
        }
        else if(setting.GetDataDel() == app::NoDel)
        {
            BadRequest(bodyMsg, event, NOTDEL);
            return true;
        }
    }
    ////////////////////////////////////////
    if (!eventStr.compare(OWNERWAIT))
    {
        std::unordered_map<std::string , DataStorage>::iterator it = data.find(nameRcv);
        if (it != data.end())
        {
            bodyMsg.erase(start, (end+10) - start);

            if ((*it).second.ExecutionEvent(event, nameRcv) == false)
            {
                (*it).second.SetWaitData(true);
                return true;
            }
            else
            {
                app::MsgError err;
                msg->AddMessage(event, err);

                if (err != app::ErrorNot)
                    wrap::Log(BADADDMSG, LOGMODULEDATA);
                else
                    (*it).second.SetWaitData(false);
            }

            return true;
        }
        else
        {
            BadRequest(bodyMsg, event, NOTUSR);
            return true;
        }
    }
    if (!eventStr.compare(DELUSR))
    {
        std::string nameRcv = bodyMsg.substr((start+9), (end - (start+9)));

        if (!data.erase(nameRcv))
        {
            BadRequest(bodyMsg, event, NOTUSR);
            return true;
        }
        else
        {
            return true;
        }
    }
    ////////////////////////////////////////
    if (!eventStr.compare(ADDUSR))
    {
        std::string nameRcv = bodyMsg.substr((start+9), (end - (start+9)));
        std::unordered_map<std::string , DataStorage>::iterator it = data.find(nameRcv);

        if (it == data.end())
        {
            int idNode = atoi(bodyMsg.substr(bodyMsg.find(IDNODESTART)+8, bodyMsg.find(IDNODEEND) ).c_str());

            data[nameRcv].SetOwnerData(idNode, nameRcv);
            return true;
        }
        else
        {
            BadRequest(bodyMsg, event, NAMEUSE);
            return true;
        }
    }
    return true;
}

void DataHandler::BadRequest(std::string& bodyMsg, app::Message& event, const char* status)
{
    size_t start;
    size_t end;

    start = bodyMsg.find(NAMESNDSTART);
    end = bodyMsg.find(NAMESNDEND);

    if (start == std::string::npos || end == std::string::npos)
    {
        wrap::Log("Error parse message\n", LOGMODULEDATA);
        return;
    }

    std::string nameRequest = bodyMsg.substr((start+9), (end - (start+9)));

    bodyMsg.erase();
    bodyMsg.append(NAMERCVSTART);
    bodyMsg.append(nameRequest);
    bodyMsg.append(NAMERCVEND);
    bodyMsg.append(DATASTART);
    bodyMsg.append(status);
    bodyMsg.append(DATAEND);

    event.SetRcv(event.GetSnd());
    event.SetSnd(app::moduleData);

    app::MsgError err;
    msg->AddMessage(event, err);

    if (err != app::ErrorNot)
        wrap::Log(status, LOGMODULEDATA);
}

// содержмое nameOwn: Onito
DataStorage::DataStorage(int idNode, std::string nameOwn)
{
    ownerName = nameOwn;
    ownerIDNetNode = idNode;
    ownerWaitData = false;
}

DataStorage::DataStorage()
{
    ownerWaitData = false;
}

DataStorage::~DataStorage()
{

}

void DataStorage::SetOwnerData(int idNode, std::string nameOwn)
{
    ownerName = nameOwn;
    ownerIDNetNode = idNode;
}

/*
#define ADDDATA "add"
#define READDATA "read"
#define WRITEDATA "write"
#define DELDATA "del"
#define OWNERWAIT "ownerwait"
*/

bool DataStorage::ExecutionEvent(app::Message& event, std::string nameRcv)
{
    std::string &bodyMsg = event.GetBodyMsg();
    size_t posFirst = bodyMsg.find(NAMESNDEND);
    size_t posSecond = bodyMsg.find(EVENTEND, posFirst);

    if ((bodyMsg.find(NAMESNDSTART) == std::string::npos) || posFirst == std::string::npos)
        return false;

    if ((bodyMsg.find(EVENTSTART, posFirst) == std::string::npos) || posSecond == std::string::npos)
        return false;

    if ((bodyMsg.find(DATASTART, posSecond) == std::string::npos) || (bodyMsg.rfind(DATAEND, posSecond) == std::string::npos))
        return false;

    std::string strEvent = bodyMsg.substr((posFirst+17), (posSecond-(posFirst+17)));

    if (!strEvent.compare(ADDDATA))
    {
        bodyMsg.erase((posFirst+10), ((posSecond+8) - (posFirst+10)));
        if (!data.empty())
            data.append("//");
        data.append(bodyMsg);

        if (ownerWaitData)
        {
            bodyMsg.erase();
            bodyMsg.append(NAMESNDSTART);
            bodyMsg.append(nameRcv);
            bodyMsg.append(NAMESNDEND);
            bodyMsg.append(NAMERCVSTART);
            bodyMsg.append(ownerName);
            bodyMsg.append(NAMERCVEND);
            bodyMsg.append(DATASTART);
            bodyMsg.append(data);
            bodyMsg.append(DATAEND);
            event.CreateMessage(bodyMsg.c_str(), ownerIDNetNode, app::moduleData);

            return true;
        }
        return true;
    }
    else if(!strEvent.compare(READDATA))
    {
        size_t start = bodyMsg.find(NAMESNDSTART);
        std::string nameRequest = bodyMsg.substr((start+9), (posFirst - (start+9)));

        bodyMsg.erase();
        bodyMsg.append(NAMESNDSTART);
        bodyMsg.append(nameRcv);
        bodyMsg.append(NAMESNDEND);
        bodyMsg.append(NAMERCVSTART);
        bodyMsg.append(nameRequest);
        bodyMsg.append(NAMERCVEND);
        bodyMsg.append(DATASTART);
        bodyMsg.append(data);
        bodyMsg.append(DATAEND);
        event.SetRcv(event.GetSnd());
        event.SetSnd(app::moduleData);
        return true;
    }
    else if(!strEvent.compare(WRITEDATA))
    {
        bodyMsg.erase((posFirst+10), (posSecond+8) - (posFirst+10));
        data = bodyMsg;

        if (ownerWaitData)
        {
            bodyMsg.erase();
            bodyMsg.append(NAMESNDSTART);
            bodyMsg.append(nameRcv);
            bodyMsg.append(NAMESNDEND);
            bodyMsg.append(NAMERCVSTART);
            bodyMsg.append(ownerName);
            bodyMsg.append(NAMERCVEND);
            bodyMsg.append(DATASTART);
            bodyMsg.append(data);
            bodyMsg.append(DATAEND);
            event.CreateMessage(bodyMsg.c_str(), ownerIDNetNode, app::moduleData);

            return true;
        }
        return true;
    }
    else if(!strEvent.compare(DELDATA))
    {
        data.erase();
        return true;
    }
    else if(!strEvent.compare(OWNERWAIT))
    {
        if (!data.empty())
        {
            bodyMsg.erase();
            bodyMsg.append(NAMESNDSTART);
            bodyMsg.append(nameRcv);
            bodyMsg.append(NAMESNDEND);
            bodyMsg.append(NAMERCVSTART);
            bodyMsg.append(ownerName);
            bodyMsg.append(NAMERCVEND);
            bodyMsg.append(DATASTART);
            bodyMsg.append(data);
            bodyMsg.append(DATAEND);
            event.CreateMessage(bodyMsg.c_str(), ownerIDNetNode, app::moduleData);

            return true;
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool DataStorage::TestWaitData()
{
    return ownerWaitData;
}

void DataStorage::SetWaitData(bool flag)
{
    ownerWaitData = flag;
}

////////////////////////////////////
}










































