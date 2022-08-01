#ifndef CACHE
#define CACHE

#include <vector>
#include <unordered_map>
#include <iostream>
#include <string>
#include "response.hpp"

class cache{
    private:
        size_t size;//how many responces can be stored in cache
        std::vector<std::string> order;
        std::unordered_map<std::string, Response> data;
    public:
        cache(size_t size):size(size){}
        void addToCache(std::string rqst_line, Response response);
        void removeFromCache(std::string rqst_line);
        void removeFromCache();
        bool checkCache(std::string rqst_line);
        void updateCache(std::string rqst_line, Response response);
        Response getResponse(std::string key);
};
#endif