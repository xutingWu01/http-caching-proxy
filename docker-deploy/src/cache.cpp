#include "cache.hpp"



void cache::addToCache(std::string rqst_line, Response response){
    if(size<=0){
        removeFromCache();
    }
    data[rqst_line] = response;
    order.push_back(rqst_line);
    size--;
}


void cache::removeFromCache(std::string rqst_line){
    data.erase(rqst_line);
    //remove from order
    for(auto it = order.begin(); it!=order.end(); ++it){
        if(rqst_line.compare(*it)==0){
            order.erase(it);
            break;
        }
    }
    size++;
}


void cache::removeFromCache(){
    std::string first = order[0];
    removeFromCache(first);
}


void cache::updateCache(std::string rqst_line, Response response){
    data[rqst_line] = response;
    for(std::vector<std::string>::iterator it = order.begin(); it!=order.end(); ++it){
        if(rqst_line.compare(*it)==0){
            order.erase(it);
            break;
        }
    }
    order.push_back(rqst_line);
}



Response cache::getResponse(std::string key){
    return (data.find(key))->second;
}


bool cache::checkCache(std::string key){
    return !(data.find(key)==data.end());
}