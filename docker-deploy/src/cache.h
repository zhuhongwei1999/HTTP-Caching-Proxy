#include <unordered_map>
#include <list>
#include <iostream>
#include <pthread.h>
#include "server.h"
#include "utils.h"
using namespace std;

extern pthread_mutex_t mutex;

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
};

#endif