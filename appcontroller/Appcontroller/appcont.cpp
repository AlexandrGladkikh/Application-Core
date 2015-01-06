#include "appcont.h"
#include <iostream>

using namespace std;

bool AppCont(app::Message& msg)
{
    const char* strRequest = msg.GetBodyMsg();
    wrap::UserDataAdd *usrData = (wrap::UserDataAdd*) strRequest;

    usrData->SetFlag(true);
    return true;
}

