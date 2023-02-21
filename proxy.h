#define BACKLOG 200
#include "client.h"
#include "server.h"

class proxy {
private:
  const char * port_num = "12345";
  int listen_fd;
public:
  proxy(const char * myport) : port_num(myport) {}
  void run();
  static void * handle_client(void * arg);
  static void handleConnect(int client_fd, int server_fd);
  static void handleGet(ClientRequest clinet_request, int client_fd, int server_fd);
  static void handlePOST(ClientRequest client_request, int client_fd, int server_fd);
  static void revalidateCachedResponse(ClientRequest client_request, int client_fd, int server_fd, ServerResponse & cached_response);
  static std::string handleChunkMessage(int server_fd, char * buffer, int buffer_size);
};