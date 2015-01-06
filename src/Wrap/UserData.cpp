#include "UserData.h"
#include <string.h>

namespace  wrap {
////////////////////////////////////

void UserDataAdd::SetName(const char *nam)
{
    if (strlen(nam) != 0)
        name = nam;
    else
        name = "";
}

void UserDataAdd::SetPos(unsigned int pos)
{
    position = pos;
}

void UserDataAdd::SetSock(unsigned int sock)
{
    socket = sock;
}

void UserDataAdd::SetPass(const char *strPass)
{
    if (strlen(strPass) != 0)
        pass = strPass;
    else
        pass = "";
}

const char* UserDataAdd::GetName()
{
    return name.c_str();
}

unsigned int UserDataAdd::GetPos()
{
    return position;
}

unsigned int UserDataAdd::GetSock()
{
    return socket;
}

const char* UserDataAdd::GetPass()
{
    return pass.c_str();
}

bool UserDataAdd::CheckFlag()
{
    return (flagAdd) ? true : false;
}

void UserDataAdd::SetFlag(bool val)
{
    flagAdd = val;
}

////////////////////////////////////
}
