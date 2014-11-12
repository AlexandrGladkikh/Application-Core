#include "AppSettings.h"
#include <iostream>
#include <fstream>
#include <string.h>

namespace app {
////////////////////////////////////

SettingData::SettingData()
{
    netProtocol = UnavailableProtocol;
    netLink = UnavailableLinked;
    dataAdd = UnavailableAdd;
    dataRead = UnavailableRead;
    dataWrite = UnavailableWrite;
    dataDel = UnavailableDel;
    valueMaxModules = 6;
}

SettingData::~SettingData()
{

}

bool SettingData::SetDataAdd(DataAdd setAdd)
{
    if (DataAddFirst <= setAdd && setAdd < DataAddEnd)
    {
        dataAdd = setAdd;
        return true;
    }
    return false;
}

bool SettingData::SetDataRead(DataRead setRead)
{
    if (DataReadFirst <= setRead && setRead < DataReadEnd)
    {
        dataRead = setRead;
        return true;
    }
    return false;
}

bool SettingData::SetDataWrite(DataWrite setWrite)
{
    if (DataWriteFirst <= setWrite && setWrite < DataWriteEnd)
    {
        dataWrite = setWrite;
        return true;
    }
    return false;
}

bool SettingData::SetDataDel(DataDel setDel)
{
    if (DataDelFirst <= setDel && setDel < DataDelEnd)
    {
        dataDel = setDel;
        return true;
    }
    return false;
}

bool SettingData::SetNetProtocol(NetProtocol setProtocol)
{
    if (NetProtocolFirst <= setProtocol && setProtocol < NetProtocolEnd)
    {
        netProtocol = setProtocol;
        return true;
    }
    return false;
}

bool SettingData::SetNetLink(NetLink setLinked)
{
    if (NetLinkFirst <= setLinked && setLinked < NetLinkEnd)
    {
        netLink = setLinked;
        return true;
    }
    return false;
}

void SettingData::SetMaxModules(int count)
{
    valueMaxModules = count;
}

void SettingData::SetHost(const char *hst)
{
    memcpy(host, hst, strlen(hst)+1);
}

void SettingData::SetServ(const char *srv)
{
    memcpy(serv, srv, strlen(srv)+1);
}

void SettingData::SetUserOnThread(int val)
{
    userOnThread = val;
}

void SettingData::SetUserOnChatRoom(int val)
{
    userOnChatRoom = val;
}

void SettingData::SetRatioAppContAppNet(int val)
{
    ratioAppContAppNet = val;
}

DataAdd SettingData::GetDataAdd()
{
    return dataAdd;
}

DataRead SettingData::GetDataRead()
{
    return dataRead;
}

DataWrite SettingData::GetDataWrite()
{
    return dataWrite;
}

DataDel SettingData::GetDataDel()
{
    return dataDel;
}

NetProtocol SettingData::GetNetprotocol()
{
    return netProtocol;
}

NetLink SettingData::GetNetLink()
{
    return netLink;
}

int SettingData::GetMaxThread()
{
    return valueMaxModules;
}

const char* SettingData::GetHost()
{
    return host;
}

const char* SettingData::GetServ()
{
    return serv;
}

int SettingData::GetUserOnThread()
{
    return userOnThread;
}

int SettingData::GetUserOnChatRoom()
{
    return userOnChatRoom;
}

int SettingData::GetRatioAppContAppNet()
{
    return ratioAppContAppNet;
}

AppSetting::AppSetting()
{

}

AppSetting::~AppSetting()
{

}

void AppSetting::SetSetting(SettingData setting)
{
    settingData = setting;
}

void AppSetting::GetSetting(SettingData &setting)
{
    setting = settingData;
}

void SettingData::ListSetting()
{
    switch(netProtocol)
    {
    case 0:
        std::cout << "IPv4" << std::endl;
    break;
    case 1:
        std::cout << "IPv6" << std::endl;
    break;
    case 2:
        std::cout << "UnavailableProtocol" << std::endl;
    break;
    }

    switch(netLink)
    {
    case 0:
        std::cout << "NetNoLinked" << std::endl;
    break;
    case 1:
        std::cout << "NetAllLinked" << std::endl;
    break;
    case 2:
        std::cout << "NetSelectiveLink" << std::endl;
    break;
    case 3:
        std::cout << "UnavailableLinked" << std::endl;
    break;
    }

    switch(dataAdd)
    {
    case 0:
        std::cout << "NoAdd" << std::endl;
    break;
    case 1:
        std::cout << "AllAdd" << std::endl;
    break;
    case 2:
        std::cout << "SelectiveAdd" << std::endl;
    break;
    }

    switch(dataRead)
    {
    case 0:
        std::cout << "NoRead" << std::endl;
    break;
    case 1:
        std::cout << "AllRead" << std::endl;
    break;
    case 2:
        std::cout << "SelectiveRead" << std::endl;
    break;
    }

    switch(dataWrite)
    {
    case 0:
        std::cout << "NoWrite" << std::endl;
    break;
    case 1:
        std::cout << "AllWrite" << std::endl;
    break;
    case 2:
        std::cout << "SelectiveWrite" << std::endl;
    break;
    }

    switch(dataDel)
    {
    case 0:
        std::cout << "NoDel" << std::endl;
    break;
    case 1:
        std::cout << "AllDel" << std::endl;
    break;
    case 2:
        std::cout << "SelectiveDel" << std::endl;
    break;
    }

    std::cout << "max modules: " << valueMaxModules << std::endl;

    std::cout << "host: " << host << std::endl;

    std::cout << "port or service: " << serv << std::endl;

    std::cout << "user on thread: " << userOnThread << std::endl;

    std::cout << "ratio AppController and AppNet: " << ratioAppContAppNet << std::endl;

    std::cout << "user on char room: " << userOnChatRoom << std::endl;
}

bool AppSetting::SaveSetting()
{
    std::fstream setting;

    setting.open("setting.DAT", std::ios::out | std::ios::binary);

    if (!setting)
    {
        return false;
    }

    setting.write(reinterpret_cast<char*>(&settingData), sizeof(settingData));

    if (!setting)
    {
        return false;
    }

    setting.close();

    return true;
}

bool AppSetting::LoadSetting()
{
    std::fstream setting;

    setting.open("setting.DAT", std::ios::in | std::ios::binary);

    setting.read(reinterpret_cast<char*>(&settingData), sizeof(settingData));

    if (!setting)
    {
        setting.close();
        return false;
    }

    setting.close();

    return true;
}

bool AppSetting::ResetSetting()
{
    settingData.SetDataAdd(AllAdd);
    settingData.SetDataDel(AllDel);
    settingData.SetNetLink(NetSelectiveLink);
    settingData.SetNetProtocol(NetIPv4);
    settingData.SetDataRead(AllRead);
    settingData.SetDataWrite(AllWrite);
    settingData.SetMaxModules(7);
    settingData.SetHost("0::0");
    settingData.SetServ("15348");
    settingData.SetRatioAppContAppNet(3);
    settingData.SetUserOnThread(5000);
    settingData.SetUserOnChatRoom(100);

    std::fstream setting;

    setting.open("setting.DAT", std::ios::out | std::ios::binary);

    if (!setting)
    {
        return false;
    }

    setting.write(reinterpret_cast<char*>(&settingData), sizeof(settingData));

    if (!setting)
    {
        return false;
    }

    setting.close();

    return true;
}

////////////////////////////////////
}































