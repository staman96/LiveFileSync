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
#include <stdlib.h>
#include <stdio.h>
#include "child.h"

using namespace std;

child::child(int buf,int myid,char* inputD,char* mirrorD,char* commonD,char* log)
:myID(myid),buffer(buf){
    int len = strlen(inputD); this->inputDir = new char[len+1]; 
    strcpy(this->inputDir,inputD);
    len = strlen(mirrorD); this->mirrorDir = new char[len+1]; 
    strcpy(this->mirrorDir,mirrorD);
    len = strlen(commonD); this->commonDir = new char[len+1]; 
    strcpy(this->commonDir,commonD);
    len = strlen(log); this->logfile = new char[len+1]; 
    strcpy(this->logfile,log);
    this->filesize = new long[1];
    *this->filesize = 0;
    this->recFilesize = 0;
    this->namelen = 0;
    signal(SIGALRM,alarmHandler);
    cout << "A child was constructed. " << endl;
}

void child::sendto(int receiverID){
    cout << "Child process " << getpid() << " with client ID " << this->myID
    << " starts sending data to " << receiverID << endl;

    alarm(30);

    /*making fifo name*/
    char* meToOther = new char[36];
    sprintf(meToOther, "%u", this->myID);//myid
    char* otherID = new char[12];
    sprintf(otherID, "%u", receiverID);
    strcat(meToOther,"_to_");//myid_to_
    strcat(meToOther,otherID);//myid_to_otherid
    delete[] otherID;
    strcat(meToOther,".fifo");//myid_to_otherid.fifo

    /*change to common directory*/
    int common = chdir(this->commonDir);
    if (common != 0){
        cout << "Error changing to common directory to create pipe from client " 
        << this->myID << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(1);
    }

    /*Creating the pipe if not already exists*/
    struct stat path_stat;
    int fifo = stat(meToOther, &path_stat);
    if(fifo == -1){
        cout << "fifo doens't exist" << endl;
        fifo = mkfifo(meToOther, 0666);
    }
    
    /*open pipe*/
    this->pipe = open(meToOther,O_WRONLY);
    if (this->pipe == -1){
        cout << "Error opening pipe." << endl;
        kill(getppid(),SIGUSR1);
        exit(10);
    }

    /*change to previous directory*/
    int prevD = chdir("..");
    if (prevD != 0){
        cout << "Error returning to starting directory to send from client " << this->myID << endl;
        kill(getppid(),SIGUSR1);
        printCurrentDir();
        exit(2);
    }

    /*opening log file to write*/
    do{
        this->lfile.open(this->logfile,std::ofstream::out | std::ofstream::app);
    }while(! this->lfile.is_open());

    /*start sending*/
    cout << "**************************************" << endl;
    send(this->inputDir);
    sendEOC();

    /*closing log file, parent is closing its ofstream in main*/
    this->lfile.close();

    common = chdir(this->commonDir);
    if (common != 0){
        cout << "Error opening common directory to delete pipe " 
        << meToOther << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(36);
    }

    close(fifo);

    /*change to previous directory*/
    prevD = chdir("..");
    if (prevD != 0){
        cout << "Error returning at starting directory" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(35);
    }

    delete[] meToOther;
    
}

void child::send(char* directory){
    alarm(0);/*canceling alarm*/

    
    int noOfEntries = countEntriesInDir(directory);
    /*changing working directory*/
    int dir = chdir(directory);
    if (dir != 0){
        cout << "Error changing directory" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(9);
    }

    /*geting all entries from current directory*/
    char** entries_array = new char*[noOfEntries];
    DIR * dirptr;
    struct dirent * entry;

    dirptr = opendir(".");
    if (dirptr == NULL){ // check if directory opened
        cout << "Could not open current directory: " << directory << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(4);
    }

    // cout << "No of entries @ " << directory << ": " << noOfEntries << endl;

    /*filling array with names of entries*/
    int namelen = 0,currentEntryIndex = 0;
    while ((entry = readdir(dirptr)) != NULL) {
        if (strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0){
            namelen = strlen(entry->d_name);
            entries_array[currentEntryIndex] = new char[namelen+1];
            strcpy(entries_array[currentEntryIndex], entry->d_name);
            // cout << "Entry[" << currentEntryIndex << "] = " 
            //     << entries_array[currentEntryIndex] << endl;
            currentEntryIndex++;
        }
    }

    /*split and send depth first*/
    for(currentEntryIndex = 0; currentEntryIndex < noOfEntries; currentEntryIndex++){
        char* currentEntry = entries_array[currentEntryIndex];
        if (child::isDIR(currentEntry) == 0){
            sendDir(currentEntry);
            send(currentEntry);
            sendDir("..");
        }
        else if (child::isDIR(currentEntry) == 1){
            sendfile(currentEntry);
        }
        else{
            cout << "Unknown file type" << endl;
        }
    }
    closedir(dirptr);
    int prevD = chdir("..");
    if (prevD != 0){
        cout << "Error returning to starting directory to send from client " << this->myID << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(12);
    }

    /*free entries array*/
    for (int i = 0; i < noOfEntries; i++){
        delete[] entries_array[i];
    }
    delete[] entries_array;
}

void child::sendfile(char* file){
    cout << "Sending file " << file << endl;
    /*writing file name length*/
    this->namelen = strlen(file) + 1;
    int wrERR = write(this->pipe, &this->namelen, 2);
    if(wrERR != 2){
        cout << "Error writing filename length " << wrERR << endl;
        kill(getppid(),SIGUSR1);
        exit(50);
    }

    /*writing filename*/
    wrERR = write(this->pipe, file, this->namelen);
    if(wrERR != this->namelen){
        cout << "Error writing filename " << wrERR << endl;
        kill(getppid(),SIGUSR1);
        exit(51);
    }

    /*writing file size*/
    struct stat st;
    stat(file, &st);
    *this->filesize = st.st_size;
    wrERR = write(this->pipe, this->filesize, 4);
    if(wrERR != 4){
        cout << "Error writing file length " << wrERR << endl;
        kill(getppid(),SIGUSR1);
        exit(52);
    }

    /*updating log file*/
    this->lfile << "NBs" << " " << this->namelen << endl;
    this->lfile.flush();

    /*writing file data to pipe*/
    int length = 0;
    char buff[this->buffer];
    int fd = open(file, O_RDONLY);
    if (fd == -1){
        cout << "Error opening file to read and send data." << endl;
        kill(getppid(),SIGUSR1);
        exit(53);
    }

    /*breaking filesize to write in pipe correct amount of bytes*/
    int no_of_reads = *this->filesize/this->buffer;
    int last_write_size = *this->filesize%this->buffer;

    cout << "_______________________________________" << endl;
    // cout << "Number of writes: " << no_of_reads << endl;
    cout << "Filesize of " << file << " is " << *this->filesize << endl;
    // cout << "Last write size: " << last_write_size << endl;
    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    fflush;

    int rdERR;
    for (int i = 0; i < no_of_reads; i++){
        /*read form pipe*/
        rdERR = read(fd, buff, this->buffer);
        if(rdERR != this->buffer){
            cout << "Error reading file data to send " << rdERR << endl;
            kill(getppid(),SIGUSR1);
            exit(87);
        }
        /*write to pipe*/
        wrERR = write(this->pipe, buff, this->buffer);
        if(wrERR != this->buffer){
            cout << "Error writing file data to pipe " << wrERR << endl;
            kill(getppid(),SIGUSR1);
            exit(54);
        }
    }

    /*last read from file with exact size left*/
    if(last_write_size != 0){
        char smallbuff[last_write_size];
        rdERR = read(fd, smallbuff, last_write_size);
        if(rdERR != last_write_size){
            cout << "Error reading last file data to send " << rdERR << endl;
            kill(getppid(),SIGUSR1);
            exit(86);
        }
        /*write last bytes to pipe*/
        wrERR = write(this->pipe, smallbuff, last_write_size);
        if(wrERR != last_write_size){
            cout << "Error writing last file data to pipe " << wrERR << endl;
            kill(getppid(),SIGUSR1);
            exit(84);
        }
    }
    close(fd);

    /*updating log file*/
    this->lfile << "FBs" << " " << *this->filesize << endl;
    this->lfile.flush();
}

void child::sendDir(char* dir){
    cout << "Sending directory " << dir << endl;

    /*writing directory name length*/
    this->namelen = strlen(dir) + 1;
    int wrERR = write(this->pipe, &this->namelen, 2);
    if(wrERR != 2){
        cout << "Error writing directory name length " << wrERR << endl;
        kill(getppid(),SIGUSR1);
        exit(55);
    }

    /*writing directory name*/
    wrERR = write(this->pipe, dir, this->namelen);
    if(wrERR != this->namelen){
        cout << "Error writing directory name " << wrERR << endl;
        kill(getppid(),SIGUSR1);
        exit(56);
    }

    if (strcmp(dir,"..") == 0) {
        *this->filesize = 2;
    }
    else{
        *this->filesize = 1;
    }
    wrERR = write(this->pipe, this->filesize, 4);
    if(wrERR != 4){
        cout << "Error writing directory type " << wrERR << endl;
        kill(getppid(),SIGUSR1);
        exit(56);
    }   

    /*updating log file*/
    this->lfile << "NBs" << " " << this->namelen << endl;
    this->lfile.flush();
}

void child::sendEOC(){
    this->namelen = 0;
    int wrERR = write(this->pipe, &this->namelen, 2);
    if(wrERR != 2){
        cout << "Error writing end of communication " << wrERR << endl;
        kill(getppid(),SIGUSR1);
        exit(75);
    }
}

void child::receivefrom(int senderID){
    cout << "Child process " << getpid() << " with client ID " << this->myID
    << " starts receiving data from " << senderID << endl;

    alarm(30);/*setting alarm*/

    /*making fifo name*/
    char* otherToMe = new char[36];
    char* myid = new char[12];
    sprintf(myid, "%u", this->myID);
    sprintf(otherToMe, "%u", senderID);
    strcat(otherToMe,"_to_");
    strcat(otherToMe,myid);
    delete[] myid;
    strcat(otherToMe,".fifo");//otherid_to_myid.fifo

    /*change to common directory*/
    int common = chdir(this->commonDir);
    if (common != 0){
        cout << "Error opening common directory to send from client " 
        << this->myID << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(6);
    }

    /*Waits for sender to create pipe*/
    struct stat path_stat;
    int fifo;
    cout << "Waiting for pipe..." << endl;
    do{
        fifo = stat(otherToMe, &path_stat);
    }while(fifo == -1);
    cout << "Pipe found" << endl;
    
    this->pipe = open(otherToMe,O_RDONLY);
    if (this->pipe == -1){
        cout << "Error opening pipe." << endl;
        kill(getppid(),SIGUSR1);
        exit(11);
    }

    /*change to starting directory*/
    int prevD = chdir("..");
    if (prevD != 0){
        cout << "Error returning starting directory" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(38);
    }

    /*opening log file to write*/
    do{
        this->lfile.open(this->logfile,std::ofstream::out | std::ofstream::app);
    }while(! this->lfile.is_open());

    /*changing to mirror directory to write*/
    int mirror = chdir(this->mirrorDir);
    if (mirror != 0){
        cout << "Error changing to mirror directory to write" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(30);
    }

    /*creating and changing to senderID directory in mirror*/
    char senderDIR[12];
    sprintf(senderDIR, "%u", senderID);
    int sendFD = mkdir(senderDIR, 0755);
    if (sendFD != 0){
        cout << "Error creating sender directory in mirror" << endl;
        kill(getppid(),SIGUSR1);
        exit(25);
    }
    sendFD = chdir(senderDIR);
    if (sendFD != 0){
        cout << "Error changing to sender directory in mirror" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(25);
    }

    cout << "Start receiving..." << endl;
    this->receive();

    mirror = chdir("..");
    if (mirror != 0){
        cout << "Error returning to mirror directory" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(28);
    }

    prevD = chdir("..");
    if (prevD != 0){
        cout << "Error returning to starting directory" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(29);
    }

    /*closing log file*/
    this->lfile.close();

    /*changing to common dir to delete pipe*/
    common = chdir(this->commonDir);
    if (common != 0){
        cout << "Error opening common directory to delete pipe " 
        << otherToMe << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(26);
    }

    close(this->pipe);
    int rm = unlink(otherToMe);
    if(rm != 0){
        cout << "Pipe: " << otherToMe << " could't be removed from PID: " 
        << getpid() << endl; 
    }

    /*change to previous directory*/
    prevD = chdir("..");
    if (prevD != 0){
        cout << "Error returning at starting directory" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(37);
    }

    delete[] otherToMe;
}

void child::receive(){
    alarm(0);/*canceling alarm*/
    int rdERR = read(this->pipe, &this->namelen, 2);
    //cout << "Received name length: " << this->namelen << endl;
    if(rdERR != 2){
        cout << "Error reading name length " << rdERR << endl;
        kill(getppid(),SIGUSR1);
        exit(50);
    }

    /*stop when data tranfer finishes*/
    while(this->namelen != 0){

        /*reading file name*/
        char name[this->namelen];
        rdERR = read(this->pipe, name, this->namelen);
        cout << "Receiving name " << name << endl;
        if(rdERR != this->namelen){
            cout << "Error reading name " << rdERR << endl;
            kill(getppid(),SIGUSR1);
            exit(60);
        }

        /*reading file size/type*/ 
        rdERR = read(this->pipe, &this->recFilesize, 4);
        if(rdERR != 4){
            cout << "Error reading filesize/type " << rdERR << endl;
            kill(getppid(),SIGUSR1);
            exit(61);
        }

        /*updating log file*/
        this->lfile << "NBr" << " " << this->namelen << endl;
        this->lfile.flush();

        if (this->recFilesize == 1){
            /*received directory*/
            receiveDir(name);
        }
        else if (this->recFilesize == 2){
            /*received to close current directory*/
            int prevD = chdir("..");
            if (prevD != 0){
                cout << "Error returning to previous directory" << endl;
                printCurrentDir();
                kill(getppid(),SIGUSR1);
                exit(62);
            }
        }
        else if (this->recFilesize > 0){
            /*received file*/
            receivefile(name);
        }
        else{
            cout << "filesize problem" << endl;
        }

        /*step: read next file name size*/
        rdERR = read(this->pipe, &this->namelen, 2);
        if(rdERR != 2){
            cout << "Error reading name length " << rdERR << endl;
            kill(getppid(),SIGUSR1);
            exit(63);
        }
    }
}

void child::receivefile(char* name){

    ofstream recFile; //creating file
    recFile.open(name); //opening file
    /*********************************
     * READS MUST NOT EXCEED FILESIZE!!!!!
     *********************************
    */
    /*breaking filesize to read correct amount of bytes*/
    char *buff = new char[this->buffer];
    int bytesLeft = this->recFilesize;
    int last_reads_size;

    cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << endl;
    cout << "Size of receiving file " << name << " is " << this->recFilesize << endl;
    cout << "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << endl;
    fflush;

    /*read file data from pipe*/
    int bytesREAD;
    bool blocked = false;
    while(bytesLeft >= this->buffer){
        bytesREAD = read(this->pipe, buff, this->buffer);
        if(bytesREAD == 0 && blocked == false){
            alarm(30);/*set alarm*/
            blocked = true;
            continue;
        }
        else if(bytesREAD == 0){
            continue;
        }
        else if(blocked){
            blocked = false;
            alarm(0);/*canceling alarm*/
        }
        /*write to file*/
        recFile.write(buff,bytesREAD);
        bytesLeft = bytesLeft - bytesREAD;
        // cout << "bytes read = " << bytesREAD << "   bytes left: " << bytesLeft << endl;
    }
    
    /*last reads form pipe with exact size left*/
    last_reads_size = bytesLeft;
    blocked = false;
    while(last_reads_size > 0){
        char* smallbuff = new char[last_reads_size];
        bytesREAD = read(this->pipe, smallbuff, last_reads_size);
        if(bytesREAD == 0 && blocked == false){
            alarm(30);/*set alarm*/
            blocked = true;
            continue;
        }
        else if(bytesREAD == 0){
            continue;
        }
        else if(blocked){
            blocked = false;
            alarm(0);/*canceling alarm*/
        }
        /*write last bytes to file*/
        recFile.write(smallbuff,bytesREAD);
        delete[] smallbuff;
        last_reads_size = last_reads_size - bytesREAD;
    }
    delete[] buff;
    recFile.close();

    /*updating log file*/
    this->lfile << "FBr" << " " << this->recFilesize << endl;
    this->lfile.flush();
}

void child::receiveDir(char* name){
    /*making received directory*/
    int err = mkdir(name, 0755);
    if (err != 0){
        cout << "Error making new directory: " << name << endl;
        kill(getppid(),SIGUSR1);
        exit(64);
    }

    /*change to new directory*/
    err = chdir(name);
    if (err != 0){
        cout << "Error changing to new directory: " << name << endl;
        kill(getppid(),SIGUSR1);
        exit(65);
    } 
}

void child::removeClientDir(int rmID){
    /*go to mirror directory*/
    int err = chdir(this->mirrorDir);
    if (err != 0){
        cout << "Error opening mirror directory to remove data of client " 
        << rmID << " from client: " << this->myID << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(15);
    }
    /*constructing client directory name in mirror directory*/
    char* clTorm = new char[12];
    sprintf(clTorm, "%u", rmID);

    /*start removing*/
    cout << "start removing mirror files of client: " << rmID << endl;
    child::removeALL(clTorm);
    delete[] clTorm;
    int prevD = chdir("..");
    if (prevD != 0){
        cout << "Error returning to previous directory" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(13);
    }
}

void child::removeALL(char *directory){
    int deldir = chdir(directory);
    if (deldir != 0){
        cout << "Error changing to next directory" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(14);
    }

    /*getting all entries from current directory*/
    DIR * dirptr;
    struct dirent * entry;

    dirptr = opendir(".");
    if (dirptr == NULL){ // check if directory opened
        cout << "Could not open current directory: " << directory << endl;
        kill(getppid(),SIGUSR1);
        exit(7);
    }

    /*deleting files and go into dir recursively depth first*/
    while ((entry = readdir(dirptr)) != NULL) {
        if (strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0){
            if (child::isDIR(entry->d_name) == 0){
                child::removeALL(entry->d_name);
            }
            else if (child::isDIR(entry->d_name) == 1){
                remove(entry->d_name);
            }
            else{
                cout << "Unknown file type to remove unable" << endl;
            }
        }
    }
    closedir(dirptr);
    int prevD = chdir("..");
    if (prevD != 0){
        cout << "Error returning to previous directory" << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(16);
    }
    prevD = rmdir(directory);
    if (prevD != 0){
        cout << "Error removing directory: " << directory << endl;
        kill(getppid(),SIGUSR1);
        printCurrentDir();
        exit(17);
    }
}

int child::isDIR(char* name){
    struct stat path_stat;
    stat(name, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int child::countEntriesInDir(char* directory){
    int counter = 0;
    DIR * dirptr;
    struct dirent * entry;

    int dir = chdir(directory);
    if (dir != 0){
        cout << "Error changing to directory " << directory << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(49);
    }

    dirptr = opendir(".");
    if (dirptr == NULL){
        cout << "Error current directory: " << directory << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(19);
    }
    while ((entry = readdir(dirptr)) != NULL) {
        counter++;
    }
    counter = counter - 2;// for "." and ".."
    closedir(dirptr);

    int prevD = chdir("..");
    if (prevD != 0){
        cout << "Error returning to previous directory" << directory << endl;
        printCurrentDir();
        kill(getppid(),SIGUSR1);
        exit(46);
    }

    return counter;
}

void child::printCurrentDir(){
    char cwd[1000];
    getcwd(cwd, sizeof(cwd));
    cout << "Current dir = " << cwd << endl;
}

void child::alarmHandler(int sig){
    cout << "Alarm caught from blocked process " << getpid() << "waiting for pipe" << endl;
    kill(getppid(),SIGUSR2);
    exit(100);
}

child::~child(){
    delete[] this->commonDir;
    delete[] this->inputDir;
    delete[] this->mirrorDir;
    delete[] this->filesize;
    delete[] this->logfile;
}