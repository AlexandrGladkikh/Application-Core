#include "UserData.h"
#include <string.h>

namespace  wrap {
////////////////////////////////////

void UserDataAdd::SetName(const char *nam)
{
        memcpy(name, nam, 100);
}

void UserDataAdd::SetPosition(unsigned int pos)
{
    position = pos;
}

void UserDataAdd::SetSock(unsigned int sock)
{
    socket = sock;
}

void UserDataAdd::SetPass(const char *strPass)
{
        memcpy(pass, strPass, 100);
}

const char* UserDataAdd::GetName()
{
    return name;
}

unsigned int UserDataAdd::GetPosition()
{
    return position;
}

unsigned int UserDataAdd::GetSock()
{
    return socket;
}

const char* UserDataAdd::GetPass()
{
    return pass;
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
