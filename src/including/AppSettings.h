#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#pragma once

#include <string>

namespace app {
////////////////////////////////////

enum NetProtocol
{
    NetProtocolFirst,
    NetIPv4 = 0,
    NetIPv6 = 1,
    UnavailableProtocol = 2,
    NetProtocolEnd = 2
};

enum NetLink
{
    NetLinkFirst,
    NetNoLinked = 0,
    NetAllLinked = 1,
    NetSelectiveLink = 2,
    UnavailableLinked = 3,
    NetLinkEnd = 3
};

enum DataAdd
{
    DataAddFirst,
    NoAdd = 0,
    AllAdd = 1,
    UnavailableAdd = 2,
    DataAddEnd = 2
};

enum DataRead
{
    DataReadFirst,
    NoRead = 0,
    AllRead = 1,
    UnavailableRead = 2,
    DataReadEnd = 2
};

enum DataWrite
{
    DataWriteFirst,
    NoWrite = 0,
    AllWrite = 1,
    UnavailableWrite = 2,
    DataWriteEnd = 2
};

enum DataDel
{
    DataDelFirst,
    NoDel = 0,
    AllDel = 1,
    UnavailableDel = 2,
    DataDelEnd = 2
};

struct SettingData
{
    NetProtocol netProtocol;
    NetLink netLink;
    DataAdd dataAdd;
    DataRead dataRead;
    DataWrite dataWrite;
    DataDel dataDel;

    int valueMaxModules;

    char host[100];
    char serv[100];

    SettingData();
    ~SettingData();

    bool SetDataAdd(DataAdd setAdd);
    bool SetDataRead(DataRead setAdd);
    bool SetDataWrite(DataWrite setAdd);
    bool SetDataDel(DataDel setDel);

    bool SetNetProtocol(NetProtocol setProtocol);
    bool SetNetLink(NetLink setLinked);

    void SetMaxModules(int count);

    void SetHost(const char *hst);
    void SetServ(const char *srv);

    DataAdd GetDataAdd();
    DataRead GetDataRead();
    DataWrite GetDataWrite();
    DataDel GetDataDel();

    NetProtocol GetNetprotocol();
    NetLink GetNetLink();

    int GetCountThread();

    const char* GetHost();
    const char* GetServ();

    void ListSetting();
};

class AppSetting
{
private:
    SettingData settingData;
public:
    AppSetting();
    ~AppSetting();

    bool SaveSetting();
    bool ResetSetting();
    bool LoadSetting();

    void SetSetting(SettingData setting);
    void GetSetting(SettingData &setting);
};

////////////////////////////////////
}

#endif // APPSETTINGS_H










































