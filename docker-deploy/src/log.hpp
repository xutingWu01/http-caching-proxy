#include <iostream>
#include <fstream>
#include <pthread.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <ctime>

class Log{
private:
    std::fstream file;
public:
    void openFile(const char* path);
    void writeLog(std::string msg);
    void closeFile(const char* path);
    char* getCurrenttime();
};