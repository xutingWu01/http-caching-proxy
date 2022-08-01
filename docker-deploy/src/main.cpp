#include "proxy.hpp"
#include "server.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
int main(){
    proxy *myproxy = new proxy("12345");
    std::cout << "start" << std::endl;
    myproxy->start();
    return EXIT_SUCCESS;
}