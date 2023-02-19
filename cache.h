#include <unordered_map>
#include "server.h"

class Cache {
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
    
  private:
    std::unordered_map<std::string, ServerResponse> cache_item;
};