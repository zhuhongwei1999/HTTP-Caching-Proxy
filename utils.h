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
#include <fstream>
#include "client.h"

void error(const char* message);
std::string get_ip_address(int socket_fd);
int get_port_num(int socket_fd);
bool is_valid_http_request(const char* buffer, int buffer_len);
ClientRequest parse_client_request(const char* buffer, int buffer_len);
int connect_to_server(const ClientRequest & request);
void sentback_request_page(ClientRequest client_request, std::fstream resource, int client_fd);
void not_found(int client_fd);