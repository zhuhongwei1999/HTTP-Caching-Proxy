#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <pthread.h>
extern std::ofstream logFile;

extern pthread_mutex_t mutex;

#ifndef SERVER_RESPONSE_H
#define SERVER_RESPONSE_H

using namespace std;

class ServerResponse {
  public:
    int status_code;
    std::string response_line;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string message;

  public:
    ServerResponse() = default;
    ServerResponse(const std::string & response_msg);
    std::vector<std::string> splitResponse(const std::string & s, const std::string & delim);
    ServerResponse & operator=(const ServerResponse & rhs) {
      if (this != &rhs) {
        response_line = rhs.response_line;
        status_code = rhs.status_code;
        headers = rhs.headers;
        body = rhs.body;
        message = rhs.message;
      }
      return *this;
    }
    bool isCacheable(int client_id, int &client_fd);
    std::time_t parseDate(const std::string & date_str);
    std::time_t parseMaxAge();
    std::time_t getExpiretime();
    std::time_t parseExpiretime();
};

#endif