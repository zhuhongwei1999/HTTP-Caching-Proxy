#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "client.h"

using namespace std;

void ClientRequest::printClientRequest() {
  cout << "id: "<< id <<endl;
  cout << "method: "<<method<<" path: "<<path<<" protocol: "<<protocol<<endl;
  for(int i = 0; i < headers.size(); i++){
    cout<< "header: "<<headers[i]<<endl;
  }
  cout<<"msg:\n"<<msg<<endl;
  cout<<"hostname: "<<host<<" port:"<<port<<endl;
}

void ClientRequest::parseRequest(const char* buffer, int buffer_len) {
  std::string message(buffer, buffer_len);
  // find the first line and split it into three parts
  msg = message;
  size_t pos1 = message.find(' ');
  size_t pos2 = message.find(' ', pos1 + 1);
  method = message.substr(0, pos1);
  std::string url = message.substr(pos1 + 1, pos2 - pos1 - 1);
  if (method == "CONNECT") {
    size_t colon_pos = url.find(":");
    path = url.substr(0, colon_pos);
    if (colon_pos != std::string::npos) {
      port = std::stoi(url.substr(colon_pos + 1));
    }
  }
  else {
    path = url;
    port = 80;
  }
  protocol = message.substr(pos2 + 1, message.find("\r\n") - pos2 - 1);
  // find the Host header and extract the host name
  size_t pos = message.find("Host: ");
  if (pos != std::string::npos) {
    std::string host = message.substr(pos + 6, message.find("\r\n", pos) - pos - 6);
    size_t colon_pos = host.find(":");
    host = host.substr(0, colon_pos);
  }
  // find the end of the headers section and extract all headers
  pos = message.find("\r\n\r\n");
  if (pos != std::string::npos) {
    std::string headers_str = message.substr(0, pos);
    size_t start = 0;
    while (true) {
      size_t end = headers_str.find("\r\n", start);
      if (end == std::string::npos) {
        break;
      }
      headers.push_back(headers_str.substr(start, end - start));
      start = end + 2;
    }
  }
}