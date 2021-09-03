#include <iostream>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include "commonIDs.h"

using namespace std;

commonIDs::commonIDs(char* foldername, char* myid){
    this->noOfIDs = 0;
    this->IDs = NULL;
    /*directory name*/
    int len = strlen(foldername); this->name = new char[len+1]; 
    strcpy(this->name,foldername);
    /*myID.id*/
    len = strlen(myid); this->myID = new char[len+1]; 
    strcpy(this->myID,myid);
    cout << "Common IDs structure was constructed." << endl;
}

char** commonIDs::checkForNewIDs(int &IDs4sync){
    IDs4sync = 0;
    char** newIDs = NULL;
    struct dirent *de;  
    DIR *dr = opendir(this->name); 
  
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        cout << "Could not open common directory" << endl; 
        return NULL; 
    } 
  
    while ((de = readdir(dr)) != NULL){

        char* currentID = de->d_name;

        if ( !this->iDexists(currentID)){
            /*checks only .id files*/
            if (strstr(currentID,".id") == NULL) continue;
            /*dont need to add itself*/
            if (strcmp(currentID,this->myID) == 0) continue;
        
            this->addID(currentID);

            /*adds id to return array dynamically*/
            IDs4sync++;
            char **temp = new char*[IDs4sync];
            int len = strlen(currentID); 
            char* newID = new char[len+1]; 
            strcpy(newID,currentID);
            temp[IDs4sync-1] = newID;
            for (int i = 0; i<IDs4sync-1;i++){
                temp[i] = newIDs[i];
            }
            delete[] newIDs;
            newIDs = temp;
        }
    } 
  
    closedir(dr);
    return newIDs;
}

char** commonIDs::checkForDepartedIDs(int &IDsToDel){
    int noOfIDfiles = countIDfiles(this->name);

    cout << "Found " << noOfIDfiles << " in common directory." << endl;
    
    IDsToDel = 0;
    char** IDs2rm = NULL;
    char** currentIDs = new char*[noOfIDfiles];
    struct dirent *entry;  
    DIR *dirptr = opendir(this->name); 
  
    if (dirptr == NULL){ // opendir returns NULL if couldn't open directory
        cout << "Could not open common directory" << endl;
        return 0; 
    } 
  
    /*getting all the .id files*/
    int namelen = 0,currentIdIndex = 0;
    while ((entry = readdir(dirptr)) != NULL) {
        if (strstr(entry->d_name,".id") != 0){/*get the .id only*/

            /*filling array*/
            namelen = strlen(entry->d_name);
            currentIDs[currentIdIndex] = new char[namelen+1];
            strcpy(currentIDs[currentIdIndex], entry->d_name);

            /*step*/
            currentIdIndex++;
        }
    }

    bool flag = false;
    /*check if saved id stil exists, if it does continue to next*/
    for(int saved = 0;saved < this->noOfIDs; saved++){
        
        for(int toRM = 0; toRM < noOfIDfiles; toRM++){
            if (strcmp(this->IDs[saved],currentIDs[toRM]) == 0){
                flag = true;
                break;
            }
        }

        if(flag == true){
            flag = false;
            continue;
        }
        else{
            cout << "Mirror data from " << this->IDs[saved] 
            << " will be deleted." << endl;
            /*add id to return array dynamically*/
            IDsToDel++;
            char **temp = new char*[IDsToDel];
            int len = strlen(this->IDs[saved]); 
            char* newID = new char[len+1]; 
            strcpy(newID,this->IDs[saved]);
            temp[IDsToDel-1] = newID;
            for (int i = 0; i<IDsToDel-1;i++){
                temp[i] = IDs2rm[i];
            }
            delete[] IDs2rm;
            IDs2rm = temp;

        }
        flag = false;
    }

    /*delete old IDs array and replace with current*/
    for(int i = 0; i<this->noOfIDs; i++){
        delete[] this->IDs[i];
    }
    delete[] this->IDs;
    this->IDs = currentIDs;
    this->noOfIDs = noOfIDfiles;

    closedir(dirptr);
    return IDs2rm;
}

int commonIDs::countIDfiles(char* directory){
    int counter = 0;
    DIR * dirptr;
    struct dirent * entry;

    dirptr = opendir(directory); 
    if (dirptr == NULL){  // opendir returns NULL if couldn't open directory 
        cout << "Could not open common directory" << endl;; 
        return 0; 
    } 
    while ((entry = readdir(dirptr)) != NULL) {
        if (strstr(entry->d_name,".id") != NULL) counter++;
    }

    closedir(dirptr);
    return counter;
}

void commonIDs::addID(char* ID){
    this->noOfIDs++;
        
    /*make new array with one extra slot*/
    char **temp = new char*[this->noOfIDs];


    int len = strlen(ID); 
    char* newID = new char[len+1]; 
    strcpy(newID,ID);

    /*pass the reference of new ID into array @ last pos*/
    temp[this->noOfIDs-1] = newID;

    /*make new array to point to old elements*/
    for (int i = 0; i<this->noOfIDs-1;i++){
        temp[i] = this->IDs[i];
    }
    
    /*free old space*/
    if(this->IDs != NULL)
        delete[] this->IDs;

    /*make->IDs pointing to new array*/
    this->IDs = temp;
}

void commonIDs::removeMyID(){
    /*change to common dir*/
    int dir = chdir(this->name);
    if (dir != 0){
        cout << "Error changing to common directory" << endl;
        return;
    }

    /*remove my.id*/
    dir = remove(this->myID);
    if (dir != 0){
        cout << "Error removing " << this->myID 
        << "from common directory." << endl;
        return;
    }

    /*go to previous dir*/
    dir = chdir("..");
    if (dir != 0){
        cout << "Error changing to previous directory" << endl;
        return;
    }
}

bool commonIDs::iDexists(char* ID){
    if (this->IDs == NULL) return false;
    for (int i = 0; i < this->noOfIDs; i++){
        if (strcmp(this->IDs[i],this->myID) == 0) continue;
        else if (strcmp(this->IDs[i],ID) == 0) return true;
    }
    return false;
}

void commonIDs::printIDs(){
    cout << "NUmber of IDs: " << this->noOfIDs << endl;
    for (int i = 0; i< this->noOfIDs; i++){
        cout << "ID[" << i << "] --> " << this->IDs[i] << endl; 
    }
}

commonIDs::~commonIDs(){
    for (int i = 0; i < this->noOfIDs; i++){
        delete[] this->IDs[i];
    }
    delete[] this->IDs;
    delete[] this->myID;
    delete[] this->name;
    cout << "Deleting commonIDs Array" << endl;
}