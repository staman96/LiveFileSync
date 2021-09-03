#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "interparent.h"
#include "child.h"

using namespace std;

interparent::interparent(int myid, child *childpr, char* log, char* com)
:myID(myid),childProcess(childpr),attempts(0){

    int len = strlen(log); this->logfile = new char[len+1]; 
    strcpy(this->logfile,log);
    len = strlen(com); this->commonDir = new char[len+1]; 
    strcpy(this->commonDir,com);
    cout << "An Intermediate parent was constructed." << endl;\
}

void interparent::childrenMaster(int clid){
    this->clientID = clid;
    while(this->attempts < 4 ){
        this->attempts++;
        this->fixpipes();
        cout << "Attempt: " << attempts << " from " << this->myID
        << " to " << this->clientID << endl;
        if(this->forkChildren()){
            /*interparent code*/
            int err = this->childrenHandler();
            if( err == 0){
                /*success*/
                this->logdata();
                break;
            }
            else if(err == 1){
                /*random error*/
                cout << "Retrying communication with client " << this->clientID << endl;
            }
            else if(err == 2){
                /*pipe error*/
                cout << "Terminating communication with client " << this->clientID << endl;
                exit (5);
            }
            else{
                cout << "Unexpected behaviour" << endl;
            }
        }
        else{
            /*children code*/
            break;
        }
    }
    
}

bool interparent::forkChildren(){
    this->senderPID = fork();
    if ( this->senderPID == -1) {
        cout << " Failed to fork sender child";
        perror("sender fork");
        exit (1) ;
    }
    if (this->senderPID == 0){
        /*child1*/
        childProcess->sendto(this->clientID);
        return false;
    }
    else{
        /*parent*/
       this->receiverPID = fork();
        if ( this->receiverPID == -1) {
            cout << " Failed to fork receiver child";
            perror("receiver fork");
            exit (2) ;
        }
        if (this->receiverPID == 0){
            /*child2*/
            childProcess->receivefrom(this->clientID);
            return false;
        }
        else{
            /*interparent*/
            return true;
        }
        
    }

}

int interparent::childrenHandler(){
    int cpid = -1,stat,reps = 0;
    bool pipeErr = false,gerror = false;
    do{
        cpid = wait(&stat);
        if(cpid == this->senderPID){
    
            if (WIFSIGNALED(stat)){
                if(WTERMSIG(stat) == SIGUSR2){
                    cout << "Sender was waiting for pipe for 30 seconds to send data to client " 
                    << this->clientID << endl;
                    pipeErr = true;
                    kill(this->receiverPID,SIGKILL);
                    waitpid(this->senderPID,NULL,WUNTRACED);
                    return 2;
                }
                else if(WTERMSIG(stat) == SIGUSR1){
                    cout << "Sender faced an unexpected error while sending data to client " 
                    << this->clientID;
                    gerror = true;
                    kill(this->receiverPID,SIGKILL);
                    waitpid(this->receiverPID,NULL,WUNTRACED);
                    return 1;
                }
                if(WTERMSIG(stat) == SIGPIPE){
                    cout << "Sender faced unexpected pipe error while sending data to client " 
                    << this->clientID << endl;
                    pipeErr = true;
                    kill(this->receiverPID,SIGKILL);
                    waitpid(this->senderPID,NULL,WUNTRACED);
                    return 1;
                }
            }
        }
        else if (cpid == this->receiverPID){
        
            if (WIFSIGNALED(stat)){
                if(WTERMSIG(stat) == SIGUSR2){
                    cout << "Receiver was waiting for pipe for 30 seconds to receive data from client " 
                    << this->clientID << endl;
                    pipeErr = true;
                    kill(this->senderPID,SIGKILL);
                    waitpid(this->senderPID,NULL,WUNTRACED);
                    return 2;
                }
                else if(WTERMSIG(stat) == SIGUSR1){
                    cout << "Receiver faced an unexpected error while receiving data from client " 
                    << this->clientID;
                    gerror = true;
                    kill(this->senderPID,SIGKILL);
                    waitpid(this->receiverPID,NULL,WUNTRACED);
                    return 1;
                }
                if(WTERMSIG(stat) == SIGPIPE){
                    cout << "Receiver faced unexpected pipe error while receiving data from client " 
                    << this->clientID << endl;
                    pipeErr = true;
                    kill(this->senderPID,SIGKILL);
                    waitpid(this->senderPID,NULL,WUNTRACED);
                    return 2;
                }
            }
        }
        else{
            cout << "Unknown child porcess error!" << endl;
            cout << "Exit code: " << WEXITSTATUS(stat) << " from PID " << cpid << endl;
        }
        
        reps++;
    }while(reps < 2);
    if (!pipeErr && !gerror){
        cout << "Data were transceived succesfully with client " << this->clientID << endl;
        return 0;
    }
    return -1;
}

void interparent::fixpipes(){
    /*making fifo name*/
    char* meToOther = new char[35];
    char* other = new char[12];
    sprintf(meToOther, "%u", this->myID);
    sprintf(other, "%u", this->clientID);

    strcat(meToOther,"_to_");/*me_to_*/
    strcat(meToOther,other);/*me_to_other*/
    strcat(meToOther,".fifo");/*me_to_other.fifo*/

    /*change to common directory*/
    int common = chdir(this->commonDir);
    if (common != 0){
        cout << "Error changing to common directory to create pipe from interparent process" << endl;
        child::printCurrentDir();
        exit(1);
    }
    

    /*deletes the pipe if already exists nad creates new*/
    struct stat path_stat;
    int fifo = stat(meToOther, &path_stat);
    if(fifo == 0){
        int rm = remove(meToOther);
        if(rm != 0){
            cout << "Pipe: " << meToOther << 
            " could't be removed from interparent with PID: " << getpid() << endl; 
        }
    }
    mkfifo(meToOther, 0666);

    

    /*change to previous directory*/
    int prevD = chdir("..");
    if (prevD != 0){
        cout << "Error returning to starting directory from interparent process" << endl;
        exit(2);
    }

    /*free heap*/
    delete[] other;
    delete[] meToOther;
}

void interparent::logdata(){
    /*parent writes to log file in ID*/
    cout << "Parent waiting for logfile to open..." << endl;
    do{
        this->lfile.open(this->logfile,std::ofstream::out | std::ofstream::app);
    }while(! this->lfile.is_open());
    cout << "Log file opened!" << endl;
    /*parent writes to log file out id*/
    this->lfile << "iID" << " " << this->clientID << endl;
    this->lfile.flush();
    this->lfile.close();/*closing log file*/
}

interparent::~interparent(){
    delete[] this->logfile;
    delete[] this->commonDir;
    cout << "interParent was destroyed." << endl;
}

