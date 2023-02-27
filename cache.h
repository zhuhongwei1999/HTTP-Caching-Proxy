#include <unordered_map>
#include <list>
#include <iostream>
#include "server.h"
using namespace std;

class Cache {
   private:
    std::unordered_map<std::string, ServerResponse> cache_item;
    std::list<std::pair<std::string, ServerResponse> > cache_list;
    size_t max_size;
    size_t size;

  public:
    void cacheResponse(const std::string & request, ServerResponse & response, int client_id) {
      auto it = response.headers.find("Expires");
      //needs seperate situation
      logFile<<client_id<<": cached, expires at "<<it->second<<endl;
      //logFile<<client_id<<": cached, but requires re-validation"<<endl;
      cache_item[request] = response;
    }

    bool getCachedResponse(const std::string & request, ServerResponse & response) {
      if (cache_item.find(request) != cache_item.end()) {
        response = cache_item[request];
        return true;
      }
      return false;
    }

    bool isEmpty() {
      return cache_item.empty();
    }
    
 
};