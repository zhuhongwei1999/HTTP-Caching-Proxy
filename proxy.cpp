#include "proxy.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "utils.h"

void proxy::run() {
  int server_fd, client_fd;
  struct sockaddr_in server_address;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  struct addrinfo * host_info_list;
  if (getaddrinfo(NULL, port_num, &hints, &host_info_list) != 0) {
    error("getaddrinfo error!");
  }
  server_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
  if (server_fd == -1) {
    error("server socket creation error!");
  }
  int yes = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (bind(server_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) != 0) {
    error("bind error!");
  }
  if (listen(server_fd, BACKLOG) != 0) {
    error("listen error!");
  }
  freeaddrinfo(host_info_list);
  // Accept
  while (1) {
    struct sockaddr_storage client_addr;
    socklen_t socket_addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &socket_addr_len);
    if (client_fd == -1) {
      error("client socket creation error!");
    }
    int id = 0;
    std::string client_ip = get_ip_address(client_fd);
    int client_port = get_port_num(server_fd);
    std::cout << client_ip << std::endl;
    char buffer[1024];
    int buffer_len = recv(client_fd, buffer, sizeof(buffer), 0);
    bool is_valid_request = is_valid_http_request(buffer, buffer_len);
    if (!is_valid_request) error("Error: Not valid HTTP request!");
    else {
      ClientRequest client_request = parse_client_request(buffer, buffer_len);
      client_request.id = id;
      id++;
      int remote_server_fd = connect_to_server(client_request);
      std::cout << remote_server_fd << std::endl;
    }
  }
}