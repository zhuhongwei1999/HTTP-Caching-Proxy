#include "proxy.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <algorithm>
#include <sys/stat.h>
#include <fstream>
#include "cache.h"

Cache cache;

void proxy::run() {
  int status;
  int server_fd, client_fd;
  int done = 1;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port_num = "12345";

  
  // struct sockaddr_in server_address;
  // struct addrinfo hints;
  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;  //allow IPv4 & IPv6   
  host_info.ai_socktype = SOCK_STREAM;//TCP
  host_info.ai_flags    = AI_PASSIVE;

  if (getaddrinfo(hostname, port_num, &host_info, &host_info_list) != 0) {
      error("getaddrinfo error!");
  }
  server_fd = socket(host_info_list->ai_family, 
            host_info_list->ai_socktype, 
            host_info_list->ai_protocol);
  if (server_fd == -1) {
    error("server socket creation error!");
  }
  int yes = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (bind(server_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) != 0) {
    error("Server bind error!");
  }
  status = listen(server_fd, BACKLOG);//BACKLOG = 200
  if (status == -1) {
    error("Server listen error!");
  }
  freeaddrinfo(host_info_list);
  // Accept
  while (true) {
    struct sockaddr_storage client_addr;
    socklen_t socket_addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &socket_addr_len);
    if (client_fd == -1) {
      error("Server accept client socket error!");
    }
    std::cout<<"Server accept client success.\n";

    int id = 0;
    std::string client_ip = get_ip_address(client_fd);
    int client_port = get_port_num(server_fd);
    char buffer[1024];
    int buffer_len = recv(client_fd, buffer, sizeof(buffer), 0);

    bool is_valid_request = is_valid_http_request(buffer, buffer_len);
    if (!is_valid_request) error("Error: Not valid HTTP request!");
    else {
      ClientRequest client_request = parse_client_request(buffer, buffer_len);
      client_request.id = id;
      id++;
      int remote_server_fd = connect_to_server(client_request);
      if (client_request.method == "CONNECT") {
        handleConnect(client_fd, remote_server_fd);
      }else if(client_request.method == "GET"){
        handleGet(client_request, client_fd, remote_server_fd);
      }else if(client_request.method == "POST"){
        //handlePost();
      }
    }
  }
}

void proxy::handleGet(ClientRequest client_request, int client_fd, int server_fd) {
  //needs cache
  send(server_fd, client_request.msg.c_str(), strlen(client_request.msg.c_str()), 0);
  char buffer[65536] = {0};
  int response_len = recv(server_fd, buffer, sizeof(buffer), 0);
  std::string response_msg(buffer);
  ServerResponse server_response(response_msg);
  if (server_response.headers.count("Transfer-Encoding") && server_response.headers["Transfer-Encoding"] == "chunked") {
    std::string chunk_size_str;
    std::string chunk_data;
    int chunk_size = 0;
    int total_size = 0;
    while (true) {
      // Read chunk size
      chunk_size_str.clear();
      while (true) {
        char c;
        recv(server_fd, &c, 1, 0);
        if (c == '\r') {
          recv(server_fd, &c, 1, 0);
          break;
        }
        chunk_size_str += c;
      }
      chunk_size = std::stoi(chunk_size_str);
      if (chunk_size == 0) {
        // Last chunk
        break;
      }
      // Read chunk data
      chunk_data.resize(chunk_size);
      int bytes_received = 0;
      while (bytes_received < chunk_size) {
        int len = recv(server_fd, &chunk_data[bytes_received], chunk_size - bytes_received, 0);
        if (len <= 0) {
          break;
        }
        bytes_received += len;
      }
      total_size += bytes_received;
      // Read trailing CRLF after chunk
      char buf[2];
      recv(server_fd, buf, 2, 0);
      // Send chunk to client
      send(client_fd, chunk_data.c_str(), chunk_data.size(), 0);
    }
    response_len = recv(server_fd, buffer, sizeof(buffer), 0);
    send(client_fd, buffer, response_len, 0);
  }
  else {
    if (server_response.isCacheable()) {
      cache.cacheResponse(client_request.headers[0], server_response);
    }

    send(client_fd, buffer, sizeof(buffer), 0);
  }
  
  return;

  // struct stat st;
  // std::string path = client_request.path;
  // if(stat(path.c_str(), &st) == -1){
  //   not_found(client_fd);
  // }else{
  //   std::fstream resource;
  //   resource.open(path.c_str());
  //  // File * resource = NULL;
  //   //resource = fopen(path, "r");
  //   if(!resource){
  //     not_found(client_fd);
  //   }
  //   char buffer[1024];
  //   if (resource.get(buffer, 1024)){
  //     int len = write(client_fd, buffer, strlen(buffer));
  //   }
  //   //sentback_request_page(client_request, resource, client_fd);
  //   resource.close();
  // }
  
}

void proxy::handleConnect(int client_fd, int server_fd) {
  std::string response = "HTTP/1.1 200 OK\r\n\r\n";
  send(client_fd, response.c_str(), response.length(), 0);
  fd_set readfds;
  while (true) {
    FD_ZERO(&readfds);
		FD_SET(server_fd, &readfds);
		FD_SET(client_fd, &readfds);
    select(std::max(server_fd, client_fd) + 1, &readfds, NULL, NULL, NULL);
    int fd[2] = {server_fd, client_fd};
    int len;
    for (int i = 0; i < 2; i++) {
      char message[65536] = {0};
      if (FD_ISSET(fd[i], &readfds)) {
        len = recv(fd[i], message, sizeof(message), 0);
        if (len < 0) return;
        else {
          if (send(fd[1 - i], message, len, 0) <= 0) return;
        }
      }
    }
  }
}


