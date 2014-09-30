 #include "../including/AppStart.h"
#include "../Modules/UI.h"
#include <iostream>

int main()
{
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
























