#include "utils.h"
void error(const char* message) {
  std::cerr << message << std::endl;
  exit(EXIT_FAILURE);
}

std::string get_ip_address(int socket_fd) {
  struct sockaddr_in socket_address;
  socklen_t address_len = sizeof(socket_address);
  char ip_address[INET_ADDRSTRLEN];
  if (getsockname(socket_fd, (struct sockaddr *)&socket_address, &address_len) == -1) {
    error("getsockname failed");
  }
  if (inet_ntop(AF_INET, &(socket_address.sin_addr), ip_address, sizeof(ip_address)) == NULL) {
    error("inet_ntop function failed");
  }
  return std::string(ip_address);
}

int get_port_num(int socket_fd) {
  struct sockaddr_in socket_address;
  socklen_t address_len = sizeof(socket_address);
  if (getsockname(socket_fd, (struct sockaddr *)&socket_address, &address_len) == -1) {
    error("getsockname failed");
  }
  return ntohs(socket_address.sin_port);
}

ClientRequest parse_client_request(const char* buffer, int buffer_len) {
  ClientRequest request;
  std::string message(buffer, buffer_len);

  // find the first line and split it into three parts
  size_t pos1 = message.find(' ');
  size_t pos2 = message.find(' ', pos1 + 1);
  request.method = message.substr(0, pos1);
  request.path = message.substr(pos1 + 1, pos2 - pos1 - 1);
  request.protocol = message.substr(pos2 + 1, message.find("\r\n") - pos2 - 1);

  // find the Host header and extract the host name
  size_t pos = message.find("Host: ");
  if (pos != std::string::npos) {
    request.ip_addr = message.substr(pos + 6, message.find("\r\n", pos) - pos - 6);
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
      request.headers.push_back(headers_str.substr(start, end - start));
      start = end + 2;
    }
  }

  return request;
}

bool is_valid_http_request(const char* buffer, int buffer_len) {
  // convert buffer to a string for easier manipulation
  std::string message(buffer, buffer_len);
  // find the end of the headers section
  size_t pos = message.find("\r\n\r\n");
  if (pos == std::string::npos) {
    return false; // no headers found
  }
  // split the message into lines
  std::stringstream ss(message);
  std::string line;
  std::vector<std::string> lines;
  while (std::getline(ss, line, '\n')) {
    lines.push_back(line);
  }
  // check the first line for a valid request method and path
  if (lines.size() < 1) {
      return false; // no request line found
  }
  std::stringstream req_line(lines[0]);
  std::string method, path, protocol;
  if (!(req_line >> method >> path >> protocol)) {
    return false; // invalid request line format
  }
  if (method != "GET" && method != "POST" && method != "CONNECT" && method != "PUT" && method != "DELETE") {
    return false; // invalid request method
  }
  if (path.empty()) {
    return false; // invalid request path
  }

  // check for the Host header
  bool host_found = false;
  for (size_t i = 1; i < lines.size(); i++) {
    if (lines[i].find("Host: ") == 0) {
      host_found = true;
      break;
    }
  }
  if (!host_found) {
    return false; // Host header not found
  }
  // check the protocol version
  if (protocol != "HTTP/1.0" && protocol != "HTTP/1.1" && protocol != "HTTP/2.0") {
    return false; // invalid protocol version
  }
  return true; // request is valid
}

int connect_to_server(const ClientRequest & request) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo * server_info_list;
  const char * remote_server_hostname = request.ip_addr.c_str();
  const char * remote_server_port = std::to_string(request.port).c_str();
  if (getaddrinfo(remote_server_hostname, remote_server_port, &hints, &server_info_list) != 0) {
    error("remote server getaddrinfo error!");
  }
  int remote_server_fd = socket(server_info_list->ai_family, server_info_list->ai_socktype, server_info_list->ai_protocol);
  if (remote_server_fd == -1) {
    error("remote server socket creation error!");
  }
  if (connect(remote_server_fd, server_info_list->ai_addr, server_info_list->ai_addrlen) == -1) {
    error("connect to server failed!");
  }
  freeaddrinfo(server_info_list);
  return remote_server_fd;
}