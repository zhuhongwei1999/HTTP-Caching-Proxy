#include "cache.h"

extern pthread_mutex_t mutex;

void Cache::cacheResponse(const std::string & request, ServerResponse & response, int client_id) {
  auto it = cache_item.find(request);
  if (it == cache_item.end()) {
    if (size >= max_size) {
      std::string key = cache_list.back();
      cache_list.pop_back();
      cache_item.erase(key);
      size--;
    }
    cache_list.push_front(request);
    cache_item[request] = std::make_pair(response, cache_list.begin());
    size++;
  }
  else {
    cache_list.erase(it->second.second);
    cache_list.push_front(request);
    it->second.second = cache_list.begin();
  }
  auto cache_control = response.headers.find("Cache-Control");
  if (cache_control != response.headers.end() && cache_control->second == "no-cache"){
    logFile << client_id << ": cached, but requires re-validation" << std::endl; 
    return;   
  }
  logFile << client_id << ": cached, expires at " << convertTimeToString(response.getExpiretime()) << std::endl;
}

bool Cache::getCachedResponse(const std::string & request, ServerResponse & response) {
  auto it = cache_item.find(request);
  if (it != cache_item.end()) {
    cache_list.erase(it->second.second);
    cache_list.push_front(request);
    it->second.second = cache_list.begin();
    response = it->second.first;
    return true;
  }
  return false;
}

bool Cache::isEmpty() {
  return cache_item.empty();
}