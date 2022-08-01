#ifndef _THREAD_INFO_H__
#define _THREAD_INFO_H__
#include "cache.hpp"
#include <pthread.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
class Thread{
public:
    cache *mycache;
    int sockect_as_server;
    std::string ip;
    int thread_id;
    Thread(cache *mycache, int server_sockect, std::string browser_ip, int id):mycache(mycache), sockect_as_server(server_sockect), ip(browser_ip), thread_id(id){}
};

#endif