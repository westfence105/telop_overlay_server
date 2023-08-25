#pragma once

#include <unordered_map>
#include <list>
#include <thread>
#include <mutex>

static constexpr int BUFSIZE = 1024;

class HttpServer {
  int m_port;

  std::unordered_map<std::string,std::string> m_variables;
  std::list<std::thread> m_threads;
  std::mutex m_mutex;

  public:
    HttpServer(int port);

    void start();
    void handle_request(int destSock);
};
