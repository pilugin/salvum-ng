#include <util/ipc.h>
#include <QtDebug>

#include <cstring>

int main(int argc, char **argv)
{
    struct A
    {
        char s[6];
        char s2[];
    };
    
    IPC::SynchroMem<A> *a = IPC::SynchroMem<A>::create("SHARED", 200);
//    strcpy(a->s2, "12345s7");
    a->s2[0] = '1';
    a->s2[1] = '2';
    
    qDebug()<< IPC::SynchroMem<A>::attach("1")->s;
    
    return 0;
}
