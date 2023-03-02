#include "utils.h"

void error(const char* message) {
  std::cerr << message << std::endl;
  exit(EXIT_FAILURE);
}

void Log(const char * message) {
  std::cout << message << std::endl;
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
  if (method != "GET" && method != "POST" && method != "CONNECT") {
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

int connect_to_server(const char * hostname, const char * port) {
  struct addrinfo hints;
  struct addrinfo * server_info_list;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(hostname, port, &hints, &server_info_list) != 0) {
    std::cerr << "remote server getaddrinfo error!" << std::endl;
    return -1;
    
  }
  int server_fd = socket(server_info_list->ai_family, server_info_list->ai_socktype, server_info_list->ai_protocol);
  if (server_fd == -1) {
    std::cerr << "remote server getaddrinfo error!" << std::endl;
    return -1;
  }
  if (connect(server_fd, server_info_list->ai_addr, server_info_list->ai_addrlen) == -1) {
    std::cerr << "connect to server failed!" << std::endl;
    return -1;
  }
  freeaddrinfo(server_info_list);
  return server_fd;
}

void sentback_request_page(ClientRequest client_request, std::fstream resource, int client_fd){
  char buffer[1024];
  if (resource.get(buffer, 1024)){
    int len = write(client_fd, buffer, strlen(buffer));
  }
}

std::string convertTimeToString(std::time_t now) {

  // std::time_t now = std::time(nullptr);
  // Convert to a struct tm object
  std::tm* timeinfo = std::localtime(&now);
  // Format the time string
  char time_str[80];
  std::strftime(time_str, 80, "%a %b %d %H:%M:%S %Y", timeinfo);
  return string(time_str);
}

void checkLogFile(std::ofstream &logFile){
  if(logFile.is_open()) {
        logFile << "This is a test log message." << std::endl;
        if (logFile.good()) {
            // Writing to the log file was successful
            //std::cout << "Successfully wrote to log file." << std::endl;
        } else {
            // Writing to the log file failed
            std::cerr << "Error writing to log file." << std::endl;
        }
        //logFile.close();
  }else{
        std::cerr << "Error opening log file." << std::endl;
  }
}