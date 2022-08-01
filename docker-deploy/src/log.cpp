#include "log.hpp"

std::mutex thread_mutex;

void openFile(const char* path){
    file.open(path, std::fstream::out);
}

void writeLog(std::string& msg){
    //need to mutex
    file<<msg;
}

void openFile(const char* path){
    file.close();
}

char* getCurrenttime(){
    time_t nowtime;
    time(&nowtime);
    char* time_str = ctime(&nowtime);
}

