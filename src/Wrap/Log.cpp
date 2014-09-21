#include "Log.h"
#include <fstream>
#include <string.h>

namespace wrap {
////////////////////////////////////

void Log(const char* log, const char* namefile)
{
    std::fstream lg;

    lg.open(namefile, std::ios::out | std::ios::app);

    lg.write(log, strlen(log));

    lg.close();
}

////////////////////////////////////
}
