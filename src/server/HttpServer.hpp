#pragma once

#include <unordered_map>
#include <list>
#include <future>
#include <mutex>
#include <nlohmann/json.hpp>

#include <HttpRequest.hpp>

static constexpr int BUFSIZE = 1024;

class HttpServer {
  int m_port;

  std::unordered_map<std::string,std::string> m_variables;
  std::list<std::future<void>> m_threads;
  std::mutex m_mutex;

  nlohmann::json m_config;

  void handleRequest(int destSock);
  void handleApi(int destSock, const HttpRequest& request);
  void handlePage(int destSock, const std::string& path);
  void handleMedia(int destSock, const std::string& path);

  std::string findMediaLibrary(const std::string& key);

  public:
    HttpServer(int port);

    void start();
};
