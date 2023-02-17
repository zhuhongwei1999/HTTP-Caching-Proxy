#include "proxy.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <algorithm>
#include "utils.h"

void proxy::run() {
  int status;
  int server_fd, client_fd;
  int done = 1;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port_num     = "666";

  
  // struct sockaddr_in server_address;
  // struct addrinfo hints;
  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;  //allow IPv4 & IPv6   
  host_info.ai_socktype = SOCK_STREAM;//TCP
  host_info.ai_flags    = AI_PASSIVE;
  hints.ai_protocol     = 0;  

  if (getaddrinfo(hostname, port_num, &hints, &host_info_list) != 0) {
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
  while (done) {
    struct sockaddr_storage client_addr;
    socklen_t socket_addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &socket_addr_len);
    if (client_fd == -1) {
      error("Server accept clinet socket error!");
    }
    cout<<"Server accept client success.\n";

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
        handleGet(client_fd);
      }
    }
  }
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
          if (send(fd[i - 1], message, len, 0) <= 0) return;
        }
      }
    }
  }
}

void proxy::handleGet(int client_fd){
  struct stat st;
  string path = client_fd.path;
  if(stat(path, &st) == -1){
    not_found(client_fd);
  }else{
    File * resource = NULL;
    resource = fopen(path, "r");
    if(resource == NULL){
      not_found(client_fd);
    }
    sentback_request_page(client_fd, resource);
    fclose(resource);
  }
}