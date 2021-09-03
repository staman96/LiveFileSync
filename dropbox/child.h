#ifndef child_h
#define child_h
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>

class child{
    private:
        int myID,buffer;
        char *inputDir,*mirrorDir,*commonDir,*logfile;
        std::ofstream lfile;
        /*protocol data*/
        short namelen;
        int pipe;
        long *filesize;
        long recFilesize;

    public:
        child(int buf,int myID, char* inputD, char* mirrorD, char* commonD, char* log);
        
        /*sender process functions*/
        void sendto(int receiverID);
        void send(char* directory);/*it is called recursively*/
        void sendfile(char* file);
        void sendDir(char* dir);
        void sendEOC();/*send end of communication*/
        
        /*receiver process functions*/
        void receivefrom(int senderID);
        void receive();
        void receivefile(char* name);/*mkfile, openfile and writetofile*/
        void receiveDir(char* name);/*mkdir and chdir*/
        
        /*remove clients mirror directory*/
        void removeClientDir(int rmID);
        static void removeALL(char *directory);/*it is called recursively*/

        /*general use functions*/
        static int isDIR(char* name);//retuns 0 if directory, 1 if file
        static int countEntriesInDir(char* directory);
        static void printCurrentDir();

        static void alarmHandler(int sig);

        ~child();
};
#endif