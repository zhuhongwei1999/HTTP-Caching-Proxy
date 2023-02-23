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
#include <thread>
#include <pthread.h>
#include "cache.h"
#include "utils.h"

Cache cache;
int id = 0;
std::ofstream logFile("/var/log/erss/proxy.log");
void proxy::run() {
  checkLogFile(logFile);
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
    pthread_t client_thread;
    pthread_create(&client_thread, NULL, handle_client, &client_fd);
  }
}

void * proxy::handle_client(void * arg) {
  int client_fd = *((int*)arg);
  char buffer[1024];
  int buffer_len = recv(client_fd, buffer, sizeof(buffer), 0);

  bool is_valid_request = is_valid_http_request(buffer, buffer_len);
  if (!is_valid_request) {
    std::cerr << "Invalid request!" << std::endl;
    return NULL;
  }
  else {
    ClientRequest client_request = parse_client_request(buffer, buffer_len);
    client_request.id = id;
    //client_request.printClientRequest();
    logFile << client_request.id <<": \""<<client_request.headers[0]
            <<"\" from"<<client_request.port<<" @ "<<getCurrentTime()<<endl;
    id++;
    int remote_server_fd = connect_to_server(client_request);
    if (client_request.method == "CONNECT") {
      handleConnect(client_fd, remote_server_fd);
    } else if (client_request.method == "GET") {
      handleGet(client_request, client_fd, remote_server_fd);
    } else if (client_request.method == "POST") {
      handlePOST(client_request, client_fd, remote_server_fd);
    }
  }
  return NULL;
}

void proxy::handleGet(ClientRequest client_request, int client_fd, int server_fd) {
  ServerResponse response;
  bool in_cache = cache.getCachedResponse(client_request.headers[0], response);
  if (in_cache) {
    std::cout << "in cache" << std::endl;
    revalidateCachedResponse(client_request, client_fd, server_fd, response);
    return;
  }
  send(server_fd, client_request.msg.c_str(), strlen(client_request.msg.c_str()), 0);
  char buffer[65536] = {0};
  int response_len = recv(server_fd, buffer, sizeof(buffer), 0);
  std::string response_msg(buffer);
  ServerResponse server_response(response_msg);
  if (server_response.headers.count("Transfer-Encoding") && server_response.headers["Transfer-Encoding"] == "chunked") {
    std::string response_body;
    response_body = handleChunkMessage(server_fd, buffer, strlen(buffer));
    send(client_fd, response_msg.c_str(), response_msg.size(), 0);
  }
  else {
    if (server_response.isCacheable()) {
      cache.cacheResponse(client_request.headers[0], server_response);
    }
    send(client_fd, buffer, sizeof(buffer), 0);
  }
  return;
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

void proxy::handlePOST(ClientRequest client_request, int client_fd, int server_fd) {
  std::string request_str = client_request.msg;
  int request_len = request_str.length();
  send(server_fd, request_str.c_str(), request_len, 0);
  char buffer[65536] = {0};
  int response_len = recv(server_fd, buffer, sizeof(buffer), 0);
  send(client_fd, buffer, sizeof(buffer), 0);
}

void proxy::revalidateCachedResponse(ClientRequest client_request, int client_fd, int server_fd, ServerResponse & cached_response) {
  std::time_t current_time = std::time(NULL);
  std::time_t max_age = cached_response.parse_max_age();
  if ((max_age >= 0 && current_time > cached_response.parse_date(cached_response.headers["Date"]) + max_age) || 
      (cached_response.headers.count("Cache-Control") && cached_response.headers["Cache-Control"] == "no-cache")) {
    // The cached response has expired, revalidate it with the server.
    ClientRequest revalidation_request = client_request;
    if (cached_response.headers.count("ETag")) {
      revalidation_request.headers.push_back("If-None-Match: " + cached_response.headers["ETag"]);
    } else if (cached_response.headers.count("Last-Modified")) {
      revalidation_request.headers.push_back("If-Modified-Since: " + cached_response.headers["Last-Modified"]);
    }
    // Send the revalidation request to the server.
    send(server_fd, revalidation_request.msg.c_str(), strlen(revalidation_request.msg.c_str()), 0);
    char buffer[65536] = {0};
    int response_len = recv(server_fd, buffer, sizeof(buffer), 0);
    std::string response_msg(buffer);
    ServerResponse revalidation_response(response_msg);
    if (revalidation_response.status_code == 304) {
      // The cached response is still valid.
      std::cout << "cached response is still valid" << std::endl;
      send(client_fd, cached_response.message.c_str(), strlen(cached_response.message.c_str()), 0);
    } else {
      // The cached response is not valid, replace it with the new response.
      std::cout << "cached response is not valid, replacing" << std::endl;
      send(client_fd, response_msg.c_str(), response_msg.size(), 0);
      cached_response = revalidation_response;
    }
  } else {
    // The cached response is still valid, return it to the client.
    std::cout << "cached response is still valid" << std::endl;
    send(client_fd, cached_response.message.c_str(), strlen(cached_response.message.c_str()), 0);
  }
}

std::string proxy::handleChunkMessage(int server_fd, char * buffer, int buffer_size) {
  std::string response;
  bool chunked_encoding = true;
  while (chunked_encoding) {
    int n = recv(server_fd, buffer, buffer_size, 0);
    std::string chunk_data(buffer, n);
    while (chunk_data.find("\r\n") != std::string::npos) {
      std::string chunk_size_str = chunk_data.substr(0, chunk_data.find("\r\n"));
      std::stringstream size_stream(chunk_size_str);
      size_t chunk_size;
      size_stream >> std::hex >> chunk_size;
      if (chunk_size == 0) {
        chunked_encoding = false;
        break;
      }
      if (chunk_data.find("\r\n\r\n") != std::string::npos) {
        response.append(chunk_data.substr(chunk_data.find("\r\n")+2, chunk_size));
        chunk_data = chunk_data.substr(chunk_data.find("\r\n")+2+chunk_size+2);
      } else {
        break;
      }
    }
  }
  return response;
}

