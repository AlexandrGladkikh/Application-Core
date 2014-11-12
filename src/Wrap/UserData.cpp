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

const char* UserDataAdd::GetName()
{
    return name.c_str();
}

unsigned int UserDataAdd::GetPos()
{
    return position;
}

bool UserDataAdd::CheckFlag()
{
    return (flagAdd) ? true : false;
}

////////////////////////////////////
}
