#ifndef MODULEDATA_H
#define MODULEDATA_H

#pragma once

#include "../including/AppData.h"
#include <unordered_map>
#include <string>
#include <queue>

namespace modules {
////////////////////////////////////

#define LOGMODULEDATA "logmoduledata.txt"

#define NAMERCVSTART    "<nameRcv>"
#define NAMERCVEND      "</nameRcv>"
#define NAMESNDSTART    "<nameSnd>"
#define NAMESNDEND      "</nameSnd>"
#define EVENTSTART      "<event>"
#define EVENTEND        "</event>"
#define DATASTART       "<data>"
#define DATAEND         "</data>"
#define IDNODESTART     "<idNode>"
#define IDNODEEND       "</idNode>"

#define ADDDATA         "add"
#define READDATA        "read"
#define WRITEDATA       "write"
#define DELDATA         "del"
#define OWNERWAIT       "ownerwait"
#define DELUSR          "delusr"
#define ADDUSR          "addusr"
#define QUIT            "quit"

/* формат сообщения для хранилища
 * <nameSnd>Zack</nameSnd><event>add</event><data>good job</data>
 * вид хранимых данных
 * <nameSnd>Zack</nameSnd><data>good job</data>
 * для разделения данных используется //
 * <nameSnd>Zack</nameSnd><data>good job</data>//<nameSnd>Zack</nameSnd><data>bad job</data}
 */

struct DataStorage
{
    int ownerIDNetNode;
    std::string ownerName;
    bool ownerWaitData;

    std::string data;

    DataStorage(int idNode, std::string nameOwn);
    DataStorage();
    ~DataStorage();

    void SetOwnerData(int idNode, std::string nameOwn);
    bool ExecutionEvent(app::Message& event, std::string nameRcv);
    bool TestWaitData();
    void SetWaitData(bool flag);
};

/* формат сообщения для handler в случае свободнго добавления данных(или любой другой операции)
 * [nameSnd]Zack[/nameSnd][nameRcv]Onito[/nameRcv][event]add[/event][data]good job[/data]
 * формат сообщения для handler в случае выборочного добавления данных(или любой другой операции)
 * [nameSnd]Zack[/nameSnd][nameRcv]Onito[/nameRcv][event]add[/event][data]good job[/data]
 * в случае выборочного добавления, перед передачей сообщения хренилищу idNode устанавливается из строки
 */

#define BADREQUEST "badrequest"
#define BADADDMSG "badaddmsg"
#define NOTADD "notadd"
#define NOTREAD "notread"
#define NOTWRITE "notwrite"
#define NOTDEL "notdel"
#define NOTUSR "notusr"
#define NAMEUSE "nameuse"

class DataHandler
{
private:
    app::SettingData setting;
    app::AppMessage *msg;

    std::unordered_map<std::string , DataStorage> data;

    void BadRequest(std::string& bodyMsg, app::Message& event, const char* status);
public:
    DataHandler(app::AppSetting *dataSttng, app::AppMessage *dataMsg);
    ~DataHandler();

    bool Handler(app::Message& event);
};

void* ModuleData(void *appData);

////////////////////////////////////
}

#endif // MODULEDATA_H
