#include <unordered_map>
#include <list>
#include "server.h"

class Cache {
   private:
    std::unordered_map<std::string, ServerResponse> cache_item;
    std::list<std::pair<std::string, ServerResponse> > cache_list;
    size_t max_size;
    size_t size;

  public:
    void cacheResponse(const std::string & request, const ServerResponse & response) {
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