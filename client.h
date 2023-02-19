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
    std::string msg;
    int port;
    std::string host;
};
