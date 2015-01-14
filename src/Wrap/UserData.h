#ifndef USERDATA_H
#define USERDATA_H

namespace wrap {
////////////////////////////////////

struct UserDataAdd
{
    bool flagAdd; // 0 -не добавлять, 1 - добавлять

    unsigned int position; // позиция в массиве client
    unsigned int socket;

    char name[20];
    char pass[20];

    UserDataAdd() { }
    ~UserDataAdd() { }

    void SetPosition(unsigned int pos);
    unsigned int GetPosition();

    void SetName(const char* nam);
    const char* GetName();

    void SetSock(unsigned int sock);
    unsigned int GetSock();

    void SetPass(const char* strPass);
    const char* GetPass();

    bool CheckFlag();
    void SetFlag(bool val);
};

}

////////////////////////////////////
#endif // USERDATA_H
