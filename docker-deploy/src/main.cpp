#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "proxy.h"

int main() {
  const char * port = "12345";
  int port_num = atoi(port);
  proxy * myproxy = new proxy(port);
  myproxy->run();
  return 0;
}
