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
    std::string method;
    std::string path;
    std::string protocol;
    std::vector<std::string> headers;
    std::string msg;
    int port;
    std::string host;
    void printClientRequest(){
      cout<<"id: "<<id<<endl;
      cout<<"method: "<<method<<" path: "<<path<<" protocol: "<<protocol<<endl;
      for(int i = 0; i < headers.size(); i++){
        cout<<"header: "<<headers[i]<<endl;
      }
      cout<<"msg: "<<msg<<endl;
      cout<<"hostname: "<<host<<" port:"<<port<<endl;
    }
};
#endif