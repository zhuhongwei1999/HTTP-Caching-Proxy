#include <iostream>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "client.h"

void error(const char* message);
std::string get_ip_address(int socket_fd);
int get_port_num(int socket_fd);
bool is_valid_http_request(const char* buffer, int buffer_len);
ClientRequest parse_client_request(const char* buffer, int buffer_len);
int connect_to_server(const ClientRequest & request);