#define BACKLOG 200
#include "utils.h"

class proxy {
private:
  const char * port_num = "12345";
  int listen_fd;
public:
  proxy(const char * myport) : port_num(myport) {}
  void run();
  void handleConnect(int client_fd, int server_fd);
  void handleGet(ClientRequest clinet_request, int client_fd, int server_fd);
};