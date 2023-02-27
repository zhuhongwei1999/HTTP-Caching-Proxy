#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#ifndef CLIENT_REQUEST_H
#define CLIENT_REQUEST_H
using namespace std;

class ClientRequest {
  public:
    int id;
    std::string msg;
    std::string method;
    std::string path;
    std::string protocol;
    std::vector<std::string> headers;
    int port;
    std::string host;

  public:
    ClientRequest(int client_id) : id(client_id), 
                                   msg(""), 
                                   method(""), 
                                   path(""), 
                                   protocol(""), 
                                   headers(std::vector<std::string>()), 
                                   port(80), 
                                   host("") {}
    void printClientRequest();
    void parseRequest(const char* buffer, int buffer_len);
};
#endif