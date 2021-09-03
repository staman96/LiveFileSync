#ifndef commonIDs_h
#define commonIDs_h
#include <iostream>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

class commonIDs{
    private:
        char* name, **IDs, *myID;//name = common dir name
        int noOfIDs;

        void addID(char* ID);
    
    public:
        commonIDs(char* foldername, char* myid);

        char** checkForNewIDs(int &IDs4sync);

        char** checkForDepartedIDs(int &IDsToDel);

        int countIDfiles(char* directory);

        /*removes my id.id from common dir*/
        void removeMyID();

        /*checks if ID.id exists*/
        bool iDexists(char* ID);

        void printIDs();

        ~commonIDs();

};
#endif