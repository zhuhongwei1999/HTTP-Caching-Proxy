#include <string>
#include <vector>
#include <map>

class ClientRequest {
  public:
    int id;
    std::string method;
    std::string path;
    std::string protocol;
    std::vector<std::string> headers;
    std::string ip_addr;
    const int port = 80;
};
