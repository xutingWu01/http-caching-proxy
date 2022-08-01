#ifndef SERVER
#define SERVER

#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <cstring>
#include <vector>
#define PORT "12345"

class server{
    private:
        //struct addrinfo hints, *servinfo;
        //int BACKLOG = 100;//how many pending connection will be held
    public:
        int setup(const char * port);
        int toAccept(int socket_fd, std::string &ip);
        int trySelect(int socket_fd);
        //char* receiveRequestFromClient(int receive_fd);
};


#endif