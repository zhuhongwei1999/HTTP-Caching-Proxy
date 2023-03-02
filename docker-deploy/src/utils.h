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
#include <ctime>
#include "client.h"

void error(const char* message);
void Log(const char * message);
std::string get_ip_address(int socket_fd);
int get_port_num(int socket_fd);
bool is_valid_http_request(const char* buffer, int buffer_len);
ClientRequest parse_client_request(const char* buffer, int buffer_len);
int connect_to_server(const char * hostname, const char * port);
void sentback_request_page(ClientRequest client_request, std::fstream resource, int client_fd);
void notFound404(int client_fd, int client_id);
void badGatway502(int cliend_fd, int client_id);
std::string convertTimeToString(std::time_t time);//std::time_t time
void checkLogFile(std::ofstream &logFile);

