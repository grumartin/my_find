#include <iostream>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <dirent.h>
#include <filesystem>
#include <stdio.h>

using namespace std;

int main(int argc, char *argv[]){
    if(argc < 3){
        cerr << "Missing arguments! Execute program with following command: \n ./myfind [-R] [-i] searchpath filename1 [filename2] …[filenameN]" << endl;
        return -1;
    }

    int c;
    bool caseInsensitive = false;
    while ((c = getopt(argc, argv, "Ri")) != EOF)
    {
        switch (c)
        {
        case 'R':       //switch to recursive mode
        case 'r':
            
            break;
        case 'I':       //switch to case-insensitive search
        case 'i':
            caseInsensitive = true;
            break;
        default:
            assert(0);
        }
    }

    const char* searchpath = argv[optind++];     //extract searchpath
    vector<const char*> filenames;

    for(; optind < argc; optind++){         //safe filenames
        filenames.push_back(argv[optind]);
    }

    struct dirent *direntp;
    DIR *dirp;
    vector<string> files;
    char dir[256];

    if ((dirp = opendir(searchpath)) == nullptr){
        cerr << "Failed to open directory" << endl;
        return -1;
    }

    while ((direntp = readdir(dirp)) != nullptr){
        for(auto & filename : filenames){
            if(caseInsensitive == true){
                if(strcasecmp(filename, direntp->d_name) == 0){
                    getcwd(dir, 256);
                    cout << "match " << filename << " " << dir << endl;
                }
            }else{
                if(strcmp(filename, direntp->d_name) == 0){
                    getcwd(dir, 256);
                    cout << "match " << filename << " " << dir << endl;
                }
            }
        }
    }
    while ((closedir(dirp) == -1) && (errno == EINTR))
        ;

    return 0;
}