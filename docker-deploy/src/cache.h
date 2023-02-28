#include <unordered_map>
#include <list>
#include <iostream>
#include <pthread.h>
#include "server.h"
#include "utils.h"

using namespace std;

#ifndef CACHE_H
#define CACHE_H

class Cache {
   private:
    std::unordered_map<std::string, std::pair<ServerResponse, std::list<std::string>::iterator> > cache_item;
    std::list<std::string> cache_list;
    size_t max_size;
    size_t size;

  public:
    Cache(size_t sz): max_size(sz), size(0) {}
    void cacheResponse(const std::string & request, ServerResponse & response, int client_id);
    bool getCachedResponse(const std::string & request, ServerResponse & response);
    bool isEmpty();
    // void cacheResponse(const std::string & request, ServerResponse & response, int client_id) {
    //   cache_item[request] = response;
    //   auto it = response.headers.find("Cache-Control");
    //   if (it != response.headers.end()) {
    //     if (it->second == "no-cache"){
    //       logFile<<client_id<<": cached, but requires re-validation"<<endl;  
    //       return;   
    //     }
    //   }
    //   //it = response.headers.find("Expires");
    //   logFile<<client_id<<": cached, expires at "<<response.parseExpiretime()<<endl;
    // }

    // bool getCachedResponse(const std::string & request, ServerResponse & response) {
    //   if (cache_item.find(request) != cache_item.end()) {
    //     response = cache_item[request];
    //     // cout<<"Test in getCcheadResponse:"<<response.message<<endl;
    //     // cout<<"Test response line: "<<response.response_line<<endl;
    //     return true;
    //   }
    //   return false;
    // }

    // void cacheResponse(const std::string & request, ServerResponse & response, int client_id) {
    //   cache_item[request] = response;
    //   auto it = response.headers.find("Cache-Control");
    //   if (it != response.headers.end()) {
    //     if (it->second == "no-cache"){
    //       logFile<<client_id<<": cached, but requires re-validation"<<endl;  
    //       return;   
    //     }
    //   }
    //   //it = response.headers.find("Expires");
    //   logFile<<client_id<<": cached, expires at "<<response.parseExpiretime()<<endl;
    // }
};

#endif