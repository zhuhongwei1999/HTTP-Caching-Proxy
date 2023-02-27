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
      cache_item[request] = response;
      auto it = response.headers.find("Cache-Control");
      if (it != response.headers.end()) {
        if (it->second == "no-cache"){
          logFile<<client_id<<": cached, but requires re-validation"<<endl;  
          return;   
        }
      }
      //it = response.headers.find("Expires");
      logFile<<client_id<<": cached, expires at "<<response.parse_expire_time()<<endl;
    }

    bool getCachedResponse(const std::string & request, ServerResponse & response) {
      if (cache_item.find(request) != cache_item.end()) {
        response = cache_item[request];
        // cout<<"Test in getCcheadResponse:"<<response.message<<endl;
        // cout<<"Test response line: "<<response.response_line<<endl;
        return true;
      }
      return false;
    }

    bool isEmpty() {
      return cache_item.empty();
    }
    
 
};