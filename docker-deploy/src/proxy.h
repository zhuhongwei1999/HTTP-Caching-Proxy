#define BACKLOG 200
#include "client.h"
#include "server.h"

extern std::ofstream logFile;

class proxy {
private:
  const char * port_num = "12345";
  int listen_fd;
public:
  proxy(const char * myport) : port_num(myport) {}
  void run();
  static void * handle_client(void * arg);
  static void handleConnect(int client_fd, int server_fd, int client_id);
  static void handleGet(ClientRequest * clinet_request, int client_fd, int server_fd);
  static void handlePOST(ClientRequest * client_request, int client_fd, int server_fd);
  static void revalidateCachedResponse(ClientRequest * client_request, int client_fd, int server_fd, ServerResponse & cached_response);
  static void handleChunkMessage(int client_fd, int server_fd);
  static int getContentLength(char * server_msg, int mes_len);
};