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

// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Cache cache(20);
int id = 0;
std::ofstream logFile("proxy.log");///var/log/erss/

void proxy::run() {
  checkLogFile(logFile);
  int status;
  int server_fd, client_fd;
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  const char * hostname = NULL;
  const char * port_num = "12345";
  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC; 
  host_info.ai_socktype = SOCK_STREAM;
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
  status = listen(server_fd, BACKLOG);
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
    // pthread_mutex_lock(&mutex);
    ClientRequest * client_request = new ClientRequest(id);
    id++;
    client_request->fd = client_fd;
    // pthread_mutex_unlock(&mutex);
    // pthread_t client_thread;
    // pthread_create(&client_thread, NULL, handle_client, client_request);
    handle_client(client_request);
  }
}

void * proxy::handle_client(void * arg) {
  ClientRequest * client_request = (ClientRequest*)arg;
  int client_fd = client_request->fd;
  char buffer[65536];
  int buffer_len = recv(client_fd, buffer, sizeof(buffer), 0);
  bool is_valid_request = is_valid_http_request(buffer, buffer_len);
  if (!is_valid_request) {
    std::cerr << "Invalid request!" << std::endl;
    return NULL;
  }
  else {
    client_request->parseRequest(buffer, buffer_len);
    client_request->printClientRequest();
    logFile << client_request->id <<": \""<<client_request->headers[0]
            <<"\" from "<<client_request->port<<" @ "<<getCurrentTime()<<std::endl;
    const char * server_hostname = client_request->host.c_str();
    const char * server_port = std::to_string(client_request->port).c_str();
    int remote_server_fd = connect_to_server(server_hostname, server_port);
    if (client_request->method == "CONNECT") {
      // pthread_mutex_lock(&mutex);
      handleConnect(client_fd, remote_server_fd, client_request->id);
      logFile << client_request->id << ": Tunnel closed" << std::endl;
      // pthread_mutex_unlock(&mutex);
    } else if (client_request->method == "GET") {
      // pthread_mutex_lock(&mutex);
      handleGet(client_request, client_fd, remote_server_fd);
      // pthread_mutex_unlock(&mutex);
    } else if (client_request->method == "POST") {
      // pthread_mutex_lock(&mutex);
      handlePOST(client_request, client_fd, remote_server_fd);
      // pthread_mutex_unlock(&mutex);
    }
    close(client_fd);
    close(remote_server_fd);
  }
  return NULL;
}

void proxy::handleGet(ClientRequest * client_request, int client_fd, int server_fd) {
  std::cout << "CLIENT_FD:" << client_fd << std::endl;
  ServerResponse response;
  bool in_cache = cache.getCachedResponse(client_request->headers[0], response);
  if (in_cache) {
    revalidateCachedResponse(client_request, client_fd, server_fd, response);
    return;
  }
  //logFile
  logFile<<client_request->id<<": not in cache"<<std::endl;
  send(server_fd, client_request->msg.c_str(), strlen(client_request->msg.c_str()), 0);
  //logFile
  logFile<<client_request->id<<": Requesting \""<<client_request->headers[0]<<"\" from "<<client_request->host<<std::endl;
  char buffer[65536] = {0};
  int response_len = recv(server_fd, buffer, sizeof(buffer), 0);
  std::string response_msg(buffer);
  ServerResponse server_response(response_msg);
   //logFile
  //logFile<<client_request.id<<": Received \""<<server_response.response_line<<"\" from "<<client_request.host<<endl;
  if (server_response.headers.count("Transfer-Encoding") && server_response.headers["Transfer-Encoding"] == "chunked") {
    std::string response_body;
    response_body = handleChunkMessage(server_fd, buffer, strlen(buffer));
    send(client_fd, response_msg.c_str(), response_msg.size(), 0);
    logFile<<client_request->id<<": Responding \""<<server_response.response_line<<"\""<<std::endl;
  }
  else {//not chunked
    if (server_response.isCacheable(client_request->id, client_fd)) {
      cache.cacheResponse(client_request->headers[0], server_response, client_request->id);
    }
    send(client_fd, buffer, sizeof(buffer), 0);
    logFile<<client_request->id<<": Responding \""<<server_response.response_line<<"\""<<std::endl;
  }
  return;
}

void proxy::handleConnect(int client_fd, int server_fd, int client_id) {
  std::string response = "HTTP/1.1 200 OK\r\n\r\n";
  std::string logResponse = "HTTP/1.1 200 OK";
  send(client_fd, response.c_str(), response.length(), 0);
  logFile<<client_id<<": Responding \""<<logResponse<<"\""<<std::endl;
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

void proxy::handlePOST(ClientRequest * client_request, int client_fd, int server_fd) {
  int request_len = client_request->getContentLength();
  if (request_len != -1) {
    send(server_fd, client_request->msg.c_str(), client_request->msg.length(), 0);
    char buffer[65536] = {0};
    int response_len = recv(server_fd, buffer, sizeof(buffer), 0);
    if (response_len != 0) {
      ServerResponse resp(buffer);
      send(client_fd, buffer, response_len, 0);
      logFile<<client_request->id<<": Responding \""<<resp.response_line<<"\""<<std::endl;
      std::cout << "Post Suceess!" << std::endl;
    }
    else {
      std::cout << "Post Failed" << std::endl;
    }
  }
}

void proxy::revalidateCachedResponse(ClientRequest * client_request, int client_fd, int server_fd, ServerResponse & cached_response) {
  //replace Time?
  std::time_t current_time = std::time(NULL);
  std::time_t expire_time = cached_response.getExpiretime();
  bool is_expired = (expire_time < current_time);
  bool is_nocache = (cached_response.headers.count("Cache-Control") && cached_response.headers["Cache-Control"] == "no-cache");
  if (is_expired || is_nocache) {
    // The cached response has expired, revalidate it with the server.
    if (is_nocache) {
      logFile << client_request->id << ": in cache, requires validation " << endl;//can expires use here?
    }
    else if (is_expired) {
      //auto it = cached_response.headers.find("Expires");
      logFile<<client_request->id<<": in cache, but expired at "<<cached_response.parseExpiretime()<<endl;
    }
   //can expires use here?
    ClientRequest revalidation_request = *client_request;
    if (cached_response.headers.count("ETag")) {
      revalidation_request.headers.push_back("If-None-Match: " + cached_response.headers["ETag"]);
    } else if (cached_response.headers.count("Last-Modified")) {
      revalidation_request.headers.push_back("If-Modified-Since: " + cached_response.headers["Last-Modified"]);
    }
    // Send the revalidation request to the server.
    send(server_fd, revalidation_request.msg.c_str(), strlen(revalidation_request.msg.c_str()), 0);
    //logFile
    logFile<<client_request->id<<": Requesting \""<<client_request->headers[0]<<"\" from "<<client_request->host<<std::endl;
    char buffer[65536] = {0};
    int response_len = recv(server_fd, buffer, sizeof(buffer), 0);
    std::string response_msg(buffer);
    ServerResponse revalidation_response(response_msg);
    //logFile
    logFile<<client_request->id<<": Received \""<<revalidation_response.response_line<<"\" from "<<client_request->host<<std::endl;
    if (revalidation_response.status_code == 304) {
      // The cached response is still valid.
      send(client_fd, cached_response.message.c_str(), strlen(cached_response.message.c_str()), 0);
      logFile<<client_request->id<<": Responding \""<<revalidation_response.response_line<<"\""<<std::endl;
    } else {
      // The cached response is not valid, replace it with the new response.
      //std::cout << "Test: cached response is not valid, replacing" << std::endl;
      //cout<<client_request.id<<": in cache, requires validation"<<endl;
      send(client_fd, response_msg.c_str(), response_msg.size(), 0);
      logFile<<client_request->id<<": Responding \""<<revalidation_response.response_line<<"\""<<std::endl;
      cached_response = revalidation_response;
    }
  } else {
    // The cached response is still valid, return it to the client.
    //std::cout << "cached response is still valid" << std::endl;
    logFile<<client_request->id<<": in cache, valid"<<std::endl;
    send(client_fd, cached_response.message.c_str(), strlen(cached_response.message.c_str()), 0);
    //cout<<"Test2:"<<cached_response.response_line<<endl;
    logFile<<client_request->id<<": Responding \""<<cached_response.response_line.c_str()<<"\""<<std::endl;
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
        response.append(chunk_data.substr(chunk_data.find("\r\n") + 2, chunk_size));
        chunk_data = chunk_data.substr(chunk_data.find("\r\n") + 2 + chunk_size + 2);
      } else {
        break;
      }
    }
  }
  return response;
}