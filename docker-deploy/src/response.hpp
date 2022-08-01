#ifndef RESPONSE
#define RESPONSE
#include <stdio.h>
#include <string.h>
#include <ctime>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>


class Response{
public:
    std::string response;
    std::string status_line;
    std::string msg_withoutline;
    std::string code;
    // header field
    std::string etag;
    std::string expire;
    std::string expire_date;
    std::string last_modify;
    std::string date;
    std::string cache_control;
    std::string max_age;
    bool is_revalidate = false;
    bool is_nocache = false;
    bool is_nostore = false;
public:
    Response(){
    }
    Response(std::string input):response(input){
        ParseStatusline();
        ParseCode();
        ParseHeader();
        getExpiretime();
    }

    void ParseStatusline();
    void ParseCode();

    std::string ParseHelper(std::string target, std::string find_start, std::string find_end);
    void ParseHeader();
    
    //handle time
    time_t getTime(std::string time_str);
    double getLifespan();
    int getLength();
    bool isChunked();
    bool isFresh();
    void getExpiretime();
};
#endif