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
    int port;
    std::string host;
};
