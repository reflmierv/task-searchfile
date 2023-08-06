#include <thread>
#include <ftw.h>
#include <dirent.h>
#include <cstring>
#include <vector>
#include <mutex>
#include <atomic>
#include <unistd.h>
#include "libsearchfile.hpp"

DIR * rootdir;

std::mutex mtx;
std::mutex first_mtx;
std::atomic_bool stopThreads;
char fileName[256];
std::string filePath;
std::vector<char *> subdirs;
int activeThreadsAmount;

int rootdirCallback(const char * fpath, const struct stat * sb, int typeflag, struct FTW * ftwbuf);
int subdirCallback(const char * fpath, const struct stat * sb, int typeflag, struct FTW * ftwbuf);
void traverseDirs();

std::string searchFile(std::string dir, std::string name, int maxThreads){
    stopThreads = false;
    activeThreadsAmount = maxThreads;
    strncpy(fileName, name.c_str(), 255);
    rootdir = opendir(dir.c_str());
    dirent * entry;
    entry = readdir(rootdir); 
    while(entry){
        if(entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
            char * dirpath = new char[257];
            strncpy(dirpath, "/", 1);
            strcat(dirpath, entry->d_name);
            subdirs.push_back(dirpath);
        }
        entry = readdir(rootdir); 
    }
    for(int i = 0; i < maxThreads; i++){
        std::thread t(traverseDirs);
        t.detach();
    }
    closedir(rootdir);
    while(!stopThreads && activeThreadsAmount){
        sleep(1);
    }
    return filePath;
}


int rootdirCallback(const char * fpath, const struct stat * sb, int typeflag, struct FTW * ftwbuf){
    if(stopThreads){
        return FTW_STOP;
    }
    if(strcmp(strchr(fpath, '/') + 1, fileName) == 0){
        filePath = std::string(fpath);
        stopThreads = true;
    }
    if(typeflag == FTW_D && strcmp(fpath, "/") != 0){
        return FTW_SKIP_SUBTREE;
    }
    else{
        return FTW_CONTINUE;
    }
}
int subdirCallback(const char * fpath, const struct stat * sb, int typeflag, struct FTW * ftwbuf){
    if(stopThreads){
        return FTW_STOP;
    }
    if(strcmp(strrchr(fpath, '/') + 1, fileName) == 0){
        filePath = std::string(fpath);
        stopThreads = true;
    }
    return FTW_CONTINUE;
}
void traverseDirs(){
    if(first_mtx.try_lock()){
        nftw("/", rootdirCallback, 256, FTW_ACTIONRETVAL);        
    }
    char * path;
    while(!stopThreads){
        mtx.lock();
        if(subdirs.empty()){
            mtx.unlock();
            break;
        }
        path = subdirs.back(); 
        subdirs.pop_back();
        mtx.unlock();
        char str[264] = "path: ";
        strcat(str, path);
        strcat(str, "\n");
        nftw(path, subdirCallback, 256, FTW_ACTIONRETVAL);
        delete[] path;
    }
    activeThreadsAmount--;
}