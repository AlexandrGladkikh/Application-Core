#ifndef USERDATA_H
#define USERDATA_H

#include <string>

namespace wrap {
////////////////////////////////////

struct UserDataAdd
{
    bool flagAdd; // 0 -не добавлять, 1 - добавлять

    unsigned int position; // позиция в массиве client

    std::string name;

    UserDataAdd() { }
    ~UserDataAdd() { }

    void SetPos(unsigned int pos);
    unsigned int GetPos();

    void SetName(const char* nam);
    const char* GetName();

    bool CheckFlag();
};

}

////////////////////////////////////
#endif // USERDATA_H
