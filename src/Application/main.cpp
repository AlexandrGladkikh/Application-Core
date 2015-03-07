#include "../including/AppStart.h"
#include "../Modules/UI.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <iostream>

int main()
{
    struct rlimit newLimit, oldLimit;
    newLimit.rlim_max = 10000;
    newLimit.rlim_cur = 9900;
    if (!setrlimit(RLIMIT_NOFILE, &newLimit))
        std::cout << oldLimit.rlim_cur << ' ' << oldLimit.rlim_max << std::endl;
    else
    {
        std::cout << "fail set descriptor" << std::endl;
        return 1;
    }

    app::AppData appData;

    if (!app::AppDataInit(appData))
    {
        std::cout << "Error init" << std::endl;
        return 1;
    }

    if (!app::StartX(&appData))
    {
        std::cout << "Error start" << std::endl;
        return 1;
    }

    if (!modules::UI(&appData))
    {
        std::cout << "Error exit app" << std::endl;
        return 1;
    }

    return 0;
}
























