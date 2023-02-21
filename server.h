#include <string>
#include <vector>
#include <map>
#include <ctime>

#ifndef SERVER_RESPONSE_H
#define SERVER_RESPONSE_H

class ServerResponse {
  public:
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string message;
  public:
    ServerResponse() = default;
    ServerResponse(const std::string & response_msg);
    std::vector<std::string> split_response(const std::string & s, const std::string & delim);
    ServerResponse & operator=(const ServerResponse & rhs) {
      if (this != &rhs) {
        status_code = rhs.status_code;
        headers = rhs.headers;
        body = rhs.body;
        message = rhs.message;
      }
      return *this;
    }
    bool isCacheable();
    std::time_t parse_date(const std::string & date_str);
    std::time_t parse_max_age();
};

#endif