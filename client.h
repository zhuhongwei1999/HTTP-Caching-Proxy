#include <string>
#include <vector>
#include <map>

#ifndef CLIENT_REQUEST_H
#define CLIENT_REQUEST_H

class ClientRequest {
  public:
    int id;
    std::string method;
    std::string path;
    std::string protocol;
    std::vector<std::string> headers;
    std::string msg;
    int port;
    std::string host;
};
#endif