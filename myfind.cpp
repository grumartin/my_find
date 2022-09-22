#include <iostream>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <dirent.h>
#include <filesystem>
#include <stdio.h>
#include "myqueue.h"

#define _DEBUG true

using namespace std;

int createQueue();
pid_t createForks(int countFiles);
void printFindings(const char* filename, filesystem::path dir);
void waitForChilds(int processNum);
void findFiles(const char* dirName, const char* filename);

bool parent = false;
bool caseInsensitive = false;
bool searchRecursive = false;

int main(int argc, char *argv[]){
    if(argc < 3){
        cerr << "Missing arguments! Execute program with following command: \n ./myfind [-R] [-i] searchpath filename1 [filename2] …[filenameN]" << endl;
        return -1;
    }

    int c;
    
    while ((c = getopt(argc, argv, "Ri")) != EOF)
    {
        switch (c)
        {
        case 'R':       //switch to recursive mode
        case 'r':
            searchRecursive = true;
            break;
        case 'I':       //switch to case-insensitive search
        case 'i':
            caseInsensitive = true;
            break;
        default:
            assert(0);
        }
    }

    char* searchpath = argv[optind++];     //extract searchpath
    vector<const char*> filenames;

    message_t msg;  /* message buffer */
    msg.mType = 1;
    int msgid = -1; /* message queue ID */
    int countFiles = 0;

    msgid = createQueue();

    for(; optind < argc; optind++){         //safe filenames
        filenames.push_back(argv[optind]);
        strncpy(msg.filename, argv[optind], MAX_DATA);
        if((msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0)) == -1){
            cerr << "Cant send message!" << endl;
            return -1;
        }
        countFiles++;
    }

    pid_t pid;
    if((pid = createForks(countFiles)) < 0){      //check for failed fork
        cerr << "Error forking!" << endl;
        return -1;
    }
    
    if(_DEBUG == true)
        cout << getpid() << " got created with parent: " << getppid() << endl; 

    if(parent == true){          //parent process waits on all childs to finish
        waitForChilds(countFiles);
        return 0;
    }

    if ((msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 0, 0)) == -1){           //receive filename from queue
        cerr << "Could not receive message from queue" << endl;
        return -1;
    }

    findFiles(searchpath, msg.filename);

    return 0;
}

void findFiles(const char* dirName, const char* filename){
    struct dirent *direntp;
    DIR *dirp;
    //char dir[256];
    
    if ((dirp = opendir(dirName)) == nullptr){       //open path
        cerr << "Failed to open directory" << endl;
        return;
    }

    while ((direntp = readdir(dirp)) != nullptr){           //read directory 
        //check if its a file or a directory
        if(direntp->d_type == DT_DIR && strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0 && searchRecursive == true){
            //if its a directory, construct new path and call function recursively
            char newpath[100] = { 0 };
            strcat(newpath, dirName);
            strcat(newpath, "/");
            strcat(newpath, direntp->d_name);
            findFiles(newpath, filename);
        }

        if(caseInsensitive == true){
            if(strcasecmp(filename, direntp->d_name) == 0){     //compare all files with filename 
                printFindings(filename, filesystem::absolute(dirName));
            }
        }else{
            if(strcmp(filename, direntp->d_name) == 0){
                printFindings(filename, filesystem::absolute(dirName));
            }
        }
    }
    closedir(dirp);       //close directory
}

void printFindings(const char* filename, filesystem::path dir){
    cout << getpid() << ": " << filename << ": " << dir << endl;
}

int createQueue(){
    /* Create new message queue */
    int msgid;
    if ((msgid = msgget(KEY, PERM | IPC_CREAT)) == -1)
    {
        /* error handling */
        cerr << "Error creating message queue" << endl;
        return -1;
    }
    return msgid;
}

pid_t createForks(int processNum){       
    if(_DEBUG == true)   
        cout << "number of processes: " << processNum << endl;
    int pids[processNum];
    for(; processNum > 0; processNum--){
        pids[processNum] = fork();

        if(pids[processNum] == 0)            //Child should not create more processes
            return getpid();        
    }
    if(_DEBUG == true)
        cout << "parent process is: " << getpid() << endl;
    parent = true;
    return getpid();
}

void waitForChilds(int processNum){
    for(; processNum > 0; processNum--){
        if(_DEBUG == true)
            cout << "Process with following pid terminated: " << wait(NULL) << endl;
        else
            wait(NULL);
    }
}