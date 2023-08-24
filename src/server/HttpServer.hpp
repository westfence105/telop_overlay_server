#pragma once

#include <unordered_map>

static constexpr int BUFSIZE = 1024;

class HttpServer {
  int m_port;

  std::unordered_map<std::string,std::string> m_variables;

  public:
    HttpServer(int port);

    void start();
    void handle_request(const std::string& requestStr, int destSock);
};
