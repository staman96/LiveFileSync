#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>
#include "commonIDs.h"
#include "child.h"
#include "interparent.h"
#include "list/list.h"

bool dir_exists(char* dir_name);
bool file_exists(char* file_name);
int extractID(char* ID_id);
void parentHandler(int sig);
void successHandler(int sig);
void transErrorHandler(int sig);
void PipeErrorHandler(int sig);

static bool terminate;

using std::cout;
using std::endl;
using std::ofstream;

int main(int argc,char* argv[]) 
{   
    cout << endl;
    terminate = false;
    List* interparentsPIDs = new List(); 
    /*parent handle*/
    signal(SIGINT,parentHandler);
    signal(SIGQUIT,parentHandler);
    /*isws balw object pou 8a 
    antisoixei se pair apo processes gia na mporw na epanalabw thn apostolh
    kai na kanw handle eykolotera*/

    int clientID = -1, buffer_size = 1000;
    char *common_dir = NULL, *input_dir = NULL, *mirror_dir= NULL, *log_file= NULL;
    bool cd = false, md = false, lf = false;/*flags to know if heap was used*/
    /*argument checking*/
    int i = 1;
    while(i<argc){
        if(strcmp(argv[i],"-c") == 0){
            //common directory
            i++;
            common_dir = argv[i];
            cout << "Common directory is: " << common_dir << endl;
        }
        else if(strcmp(argv[i],"-i") == 0){
           //input directory
           i++;
           input_dir = argv[i];
           cout << "Input directory is: " << input_dir << endl;
        }
        else if(strcmp(argv[i],"-m") == 0){
           //mirror directory
           i++;
           mirror_dir = argv[i];
           cout << "Mirror directory is: " << mirror_dir << endl;
        }
        else if (strcmp(argv[i],"-l") == 0) {
            //log file
            i++;
            log_file = argv[i];
            cout << "Log file is: " << log_file << endl;
        }
        else if (strcmp(argv[i], "-n") == 0) {
            //clientID
            i++;
            clientID = atoi(argv[i]);
            cout << "ID is: " << clientID << endl;
        }
        else if (strcmp(argv[i],"-b") == 0) {
            //buffer_size
            i++;
            buffer_size = atoi(argv[i]);
            cout << "Buffer size is: " << buffer_size << endl;
        }
        else{
            cout << "Invalid parameter" << endl;
        }

        i++;
    }


    /*checks if all file and folder arguments were given*/
    //common directory
    if (common_dir == NULL){
        cout << "Making common directory because it wasn't given." << endl;
        common_dir = new char[11];
        cd = true;
        strcpy(common_dir,"common_dir");
        int cdir = mkdir(common_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if(cdir != 0){
            cout << "Error 1 making common directory." << endl;
            return 1;
        }
    }
    else if (!dir_exists(common_dir)){
        cout << "Making common directory because given doesn't exist." << endl;
        int cdir = mkdir(common_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        cout << cdir << " common dir return." << endl;
        if(cdir != 0){
            cout << "Error 2 making common directory." << endl;
            return 2;
        }
    }
    //input directory
    if (input_dir == NULL){
        cout << "Input directory argument wasn't given." << endl
        << "Terminating process..." << endl;;
        return 3;
    }
    else if (!dir_exists(input_dir)){
        cout << "Input directory given doesn't exist." << endl
        << "Terminating process..." << endl;
        return 4;
    }
    //mirror directory
    if (mirror_dir == NULL){
        cout << "Making mirror directory because it wasn't given." << endl;
        char* clid = new char[12];
        sprintf(clid, "%u", clientID);
        mirror_dir = new char[24];
        strcpy(mirror_dir,"mirror_dir");
        strcat(mirror_dir,clid);
        delete[] clid;
        md = true;
        int mdir = mkdir(mirror_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if(mdir != 0){
            cout << "Error 1 making mirror directory " << mirror_dir << endl;
            delete[] mirror_dir;
            return 5;
        }
    }
    else if (dir_exists(mirror_dir)){
        cout << "Mirron directory is already being used." << endl;;
        return 6;
    }
    else{
        cout << "Making given mirror directory because it doesn't exist." << endl;
        int mdir = mkdir(mirror_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        cout << mdir << " mirror dir" << endl;
        if(mdir != 0){
            cout << "Error 2 making mirror directory " << *mirror_dir << endl;
            return 7;
        }
    }
    if (log_file == NULL){
        cout << "Setting Log file name because it wasn't given." << endl;
        char* clid = new char[12];
        sprintf(clid, "%u", clientID);
        log_file = new char[22];
        strcpy(log_file,"log_file");
        strcat(log_file,clid);
        delete[] clid;
        lf = true;
    }

    /*getting proccess ID*/
    int pid = getpid();
    cout << "Proccess ID: " << pid << endl;
    if(clientID < 0){
        cout << "Client ID value is invalid, it has to be a positive integer." 
        << endl << "Giving process id as client to continue." << endl;
    }

    /*making string of file name and path */
    char* cl_id = new char[16];
    sprintf(cl_id, "%u", clientID);
    strcat(cl_id,".id");
    char fileID[80];
    strcpy(fileID,common_dir);
    strcat(fileID,cl_id);
    cout << fileID << endl;

    /*checking if file id.id exists*/
    if (file_exists(fileID)) {
        cout << fileID << " exists" << endl
        << "Terminating proccess" << endl;
        return 8;
    }

    /*writing proccess id to file*/
    ofstream common;
    common.open(fileID);
    common << pid;
    common.close();

    /*Initializations*/
    commonIDs* clientIDs = new commonIDs(common_dir, cl_id);
    ofstream lfile;/*for log file*/
    

    /*proccessing new IDs*/
    bool t = true;
    char** IDs = NULL;
    int IDsCounter, timepassed = 0;
    child* childProcess 
        = new child(buffer_size,clientID,input_dir,mirror_dir,common_dir,log_file);
    interparent* intermediate 
        = new interparent(clientID,childProcess, log_file, common_dir);

    /*loop runs only in parent until signal to stop arrives*/
    while(terminate == false){
        IDs = NULL;
        IDsCounter = 0;
        
        /*checks for new ids*/
        IDs = clientIDs->checkForNewIDs(IDsCounter);

        if (IDsCounter > 0){
            for(int syncedIDs = 0; syncedIDs < IDsCounter; syncedIDs++){

                
                int interparPID = fork();
                if ( interparPID == -1) {
                    cout << " Failed to fork ";
                    perror("fork1");
                    exit (1) ;
                }
                if (interparPID == 0){
                    /*intermediate parent*/
                    intermediate->childrenMaster(extractID(IDs[syncedIDs]));
                    break;
                }
                else{
                    /*root process*/
                    
                    /*add intermediate PID to list, to use it upon deletion*/
                    int* interPID = new int;/*pointer for list*/
                    *interPID = interparPID;
                    interparentsPIDs->pushBack(interPID);
                }
                
            }
        }

        /*delete array of new clients*/
        for (int i = 0; i < IDsCounter; i++){
            delete[] IDs[i];
        }
        if (IDs != NULL){
            delete[] IDs;
            IDs = NULL;
        }

        /*if child process break loop*/
        if (getpid() != pid) break;

        /*check for clients that left common directory*/
        IDs = clientIDs->checkForDepartedIDs(IDsCounter);

        if (IDsCounter > 0){
            for(int rmIDs = 0; rmIDs < IDsCounter; rmIDs++){
                pid_t rmpid = fork();
                if ( rmpid == -1) {
                    cout << " Failed to fork ";
                    perror("fork");
                    exit (10) ;
                }
                if (rmpid == 0){
                    /*child*/
                    childProcess->removeClientDir(extractID(IDs[rmIDs]));
                    break;
                }
                else{
                    /*opening log file to write*/
                    cout << "Parent waiting for logfile to open..." << endl;
                    do{
                        lfile.open(log_file,std::ofstream::out | std::ofstream::app);
                    }while(! lfile.is_open());
                    cout << "Log file opened!" << endl;
                    /*parent writes to log file out id*/ 
                    lfile << "oID" << " " << extractID(IDs[rmIDs]) << endl;
                    lfile.flush();
                    lfile.close();/*closing log file*/
                }
                /*waiting for child process to end and check if succeded, 
                and deletes objects*/
                cout << "Waiting for data removal of client " 
                    << extractID(IDs[rmIDs])  << endl;
                waitpid(rmpid,NULL,WUNTRACED);
            }
        }

        /*delete array of departed clients*/
        for (int i = 0; i < IDsCounter; i++){
            delete[] IDs[i];
        }
        if (IDs != NULL){
            delete[] IDs;
        }

        /*periodically checking*/
        sleep(1);
        timepassed++;
        cout << timepassed << " seconds passed.(PID: " << getpid() << ")" << endl;
    }
    
    if(pid != getpid()){
        for (int i = 0; i < interparentsPIDs->size(); i++)
        {
            delete(interparentsPIDs->get(i));/*free data pointer*/
        }
    }
    

    /*operations executed from parent on exit*/
    if (pid == getpid()){
        cout << "Client " << clientID << " with Process ID " << getpid() 
            << " terminating and printing clientele data..." << endl;
        clientIDs->printIDs();
        clientIDs->removeMyID();/*remove myid.id*/
        child::removeALL(mirror_dir);/*empty and remove my mirror directory*/

        int stat;
        int curr_pid;
        for (int i = 0; i < interparentsPIDs->size(); i++)
        {
            curr_pid = *(interparentsPIDs->get(i));
            cout << "##################  " << curr_pid << endl;
            if(waitpid(curr_pid,&stat, WNOHANG) == 0){
                /*killing unfinished child*/
                cout << "Killing intermediate parent with PID " << curr_pid << endl;
                kill(curr_pid,SIGTERM);
            }
            delete(interparentsPIDs->get(i));/*free data pointer*/
        }
    }
    

    /*free space*/
    delete[] cl_id;
    delete clientIDs;
    delete childProcess;
    delete intermediate;
    delete interparentsPIDs;
    if(md) delete[] mirror_dir;
    if(cd) delete[] common_dir;
    if(lf) delete[] log_file;

    return 0;
}


/*checks if directory exists, returns true if exists, else false*/
bool dir_exists(char* dir_name){
    DIR* dir = opendir(dir_name);
    if (dir){
        /* Directory exists. */
        closedir(dir);
        return true;
    }
    else if (ENOENT == errno)
        /* Directory does not exist. */
        return false;
    else{
        cout << "Directory failed for an unexpected reason." << endl;
        return false;
        /* opendir() failed for some other reason. */
    }
}

/*checks if directory exists, returns true if exists, else false*/
bool file_exists(char* file_name){
    FILE* file = fopen(file_name,"r");
    if (file){
        /* File exists. */
        fclose(file);
        return true;
    }
    else{
        return false;
        /* opendir() failed for some other reason. */
    }
}

int extractID(char* ID_id){
    int len = strlen(ID_id);
    char* ID = new char[len-2];
    strncpy(ID,ID_id,len-2);
    int IDval = atoi(ID);
    delete[] ID;
    return IDval;
}

void parentHandler(int sig){
    cout << "Caught parent signal: " << sig << endl;
    cout << "Terminating parent process.." << endl;
    /*if true, main exits the loop after current cycle and exits after children
    finish and memory was freed*/  
    terminate = true;
}