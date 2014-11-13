#ifndef USERDATA_H
#define USERDATA_H

#include <string>

namespace wrap {
////////////////////////////////////

struct UserDataAdd
{
    bool flagAdd; // 0 -не добавлять, 1 - добавлять

    unsigned int position; // позиция в массиве client
    unsigned int socket;

    std::string name;
    std::string pass;

    UserDataAdd() { }
    ~UserDataAdd() { }

    void SetPos(unsigned int pos);
    unsigned int GetPos();

    void SetName(const char* nam);
    const char* GetName();

    void SetSock(unsigned int sock);
    unsigned int GetSock();

    void SetPass(const char* strPass);
    const char* GetPass();

    bool CheckFlag();
};

}

////////////////////////////////////
#endif // USERDATA_H
