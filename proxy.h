#define BACKLOG 200

class proxy {
private:
  const char * port_num = "12345";
  int listen_fd;
public:
  proxy(const char * myport) : port_num(myport) {}
  void run();
};