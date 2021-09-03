#ifndef interparent_h
#define interparent_h
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <iostream>
#include <string.h>
#include "child.h"

class interparent
{
    private:
        pid_t senderPID, receiverPID;
        int myID, clientID, attempts;
        child *childProcess;
        char* logfile,* commonDir;
        std::ofstream lfile;

    public:
        interparent(int myid, child *childpr, char* log, char* com);
        /*overwatches children*/
        void childrenMaster(int clid);
        /*forks children to send and receive data,
        returns if it is interparent process*/
        bool forkChildren();
        /*waits for children and checks signals*/
        int childrenHandler();
        /*deleting pipes if already exist and making new*/
        void fixpipes();
        /*logs data if finished successfully*/
        void logdata();

        ~interparent();
};

#endif