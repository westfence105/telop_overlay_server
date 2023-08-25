#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <functional>
#include <filesystem>

#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#include "HttpServer.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "../util/PNG.hpp"

using namespace std::literals::string_literals;
using namespace std::chrono_literals;
using json = nlohmann::json;

HttpServer::HttpServer(int port) {
  m_port = port;

  std::ifstream configFile("server_config.json");
  if (configFile) {
    m_config = json::parse(configFile);
  }
}

void HttpServer::start() {
  int status;
  int srcSock, destSock;
  struct sockaddr_in srcAddr, destAddr;
  socklen_t destAddrSize;

  srcAddr.sin_port = htons(m_port);
  srcAddr.sin_family = AF_INET;
  srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  srcSock = socket(AF_INET, SOCK_STREAM, 0);
  struct timeval timeout;
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;
  status = setsockopt(srcSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
  for (int i = 0; i < 3; ++i) {
    if ((status = bind(srcSock, (struct sockaddr*)&srcAddr, sizeof(srcAddr))) == 0) {
      break;
    }
  }
  if (status) {
    std::cerr << "Error while binding socket." << std::endl;
    return;
  }

  bool loop = true;
  std::thread keyWatch([&] () {
    char buf;
    std::cout << "Server running on port " << m_port << "...\nPress 'x' to stop." << std::endl;
    do {
      std::cin >> buf;
    } while (buf != 'x');
    loop = false;
  });

  listen(srcSock, 100);
  while (loop) {
    destSock = accept(srcSock, (struct sockaddr*)&destAddr, &destAddrSize);
    if (destSock >= 0) {
      m_threads.push_back(std::async(std::launch::async, std::bind(&HttpServer::handleRequest, this, destSock)));
    }
  }

  keyWatch.join();
  for (auto& t : m_threads) {
    if (t.wait_for(0ms) != std::future_status::ready) {
      std::cout << "Waiting connection closed..." << std::endl;
      t.wait();
    }
    t.get();
  }

  close(srcSock);
}

void HttpServer::handleRequest(int destSock) {
  size_t recvSize;
  char buf[BUFSIZE+1];
  std::ostringstream oss;
  do {
    memset(buf, 0, BUFSIZE+1);
    recvSize = recv(destSock, buf, BUFSIZE, 0);
    oss << buf;
  } while (recvSize >= BUFSIZE);

  HttpRequest request(oss.str());

  std::string path = request.path();
  if (path.starts_with("/api/")) {
    handleApi(destSock, request);
  }
  else if (path.starts_with("/media/")) {
    handleMedia(destSock, path.substr(7));
  }
  else {
    handlePage(destSock, path.substr(1));
  }

  close(destSock);
}

void HttpServer::handlePage(int destSock, const std::string& path) {
  std::string filePath;
  std::string ext = "html";
  size_t extPos = path.find_first_of('.');
  if (extPos == std::string::npos) {
    if (path.find_first_of('/') == std::string::npos) {
      filePath = "pages/"s + path + "/index.html";
    }
    else {
      filePath = "pages/"s + path + ".html";
    }
  }
  else if (extPos+1 < path.length()) {
    ext = path.substr(extPos+1);
    filePath = "pages/"s + path;
  }

  std::ifstream file(filePath, std::ios::in);
  if (file) {
    HttpResponse(file, ext).send(destSock);
  }
  else {
    HttpResponse(404).send(destSock);
  }
}

static std::vector<std::string> splitPath(const std::string& path) {
  std::vector<std::string> parsedPath;
  size_t offset = 0;
  while (offset < path.length()) {
    size_t pos = path.find('/', offset);
    if (pos == std::string::npos) {
      parsedPath.push_back(path.substr(offset));
      break;
    }
    else {
      parsedPath.push_back(path.substr(offset, pos - offset));
      offset = pos + 1;
    }
  }

  return parsedPath;
}

static std::string joinPath(const std::vector<std::string>& path) {
  if (path.size() == 0) {
    return "";
  }
  else {
    std::ostringstream oss;
    for (size_t i = 0; i < path.size() - 1; ++i) {
      oss << path[i] << "/";
    }
    oss << path[path.size()-1];
    return oss.str();
  }
}

std::string HttpServer::findMediaLibrary(const std::string& key) {
  if (m_config.is_object() && m_config.count("media") && m_config["media"].is_object() && m_config["media"].count(key)) {
    return m_config["media"][key];
  }
  else {
    return "";
  }
}

void HttpServer::handleMedia(int destSock, const std::string& path) {
  std::vector<std::string> parsedPath = splitPath(path);
  std::string library;
  if (parsedPath.size() < 2 || (library = findMediaLibrary(parsedPath[0])) == "") {
    HttpResponse(404).send(destSock);
  }
  else {
    parsedPath[0] = library;

    std::string ext;
    std::string& filename = parsedPath[parsedPath.size()-1];
    size_t p;
    size_t offset = 0;
    bool decoded = false;
    while (offset < filename.size() && (p = filename.find_first_of('%', offset)) != std::string::npos) {
      if (p < filename.size() - 2) {
        std::string codeStr = filename.substr(p + 1, 2);
        try {
          unsigned int code = std::stoul(codeStr, NULL, 16);
          filename.replace(p, 3, 1, (char)code);
          offset = p + 1;
        }
        catch (std::invalid_argument e) {
          std::cerr << e.what() << std::endl;
          offset = p + 3;
          continue;
        }
      }
    }
    p = filename.find_first_of('.');
    if (p != std::string::npos && p < filename.size() - 1) {
      ext = filename.substr(p+1);
    }

    std::ifstream file(joinPath(parsedPath), std::ios::in|std::ios::binary);
    if (file) {
      HttpResponse(file, ext).send(destSock);
    }
  }
}

void HttpServer::handleApi(int destSock, const HttpRequest& request) {
  std::vector<std::string> parsedPath = splitPath(request.path().substr(5));

  if (parsedPath.size() == 2 && parsedPath[0] == "variable") {
    std::unique_lock lock(m_mutex);
    if (request.type() == HttpRequestType::POST) {
      m_variables[parsedPath[1]] = request.body();
    }
    HttpResponse(m_variables[parsedPath[1]]).send(destSock);
  }
  else if (parsedPath.size() == 2 && parsedPath[0] == "media") {
    if (parsedPath[1] == "search") {
      json reqData = json::parse(request.body());
      if (!reqData.is_object() || !reqData.count("library")) {
        HttpResponse(400).send(destSock);
        return;
      }

      std::string library = findMediaLibrary(reqData["library"]);
      if (library == "") {
        HttpResponse(404).send(destSock);
        return;
      }

      std::list<std::string> files;
      for (const auto& file : std::filesystem::directory_iterator(library)) {
        if (!file.is_regular_file()) continue;
        std::string path = file.path();

        std::string parameters;
        if (path.size() > 4 && path.substr(path.size()-4) == ".png") {
          std::ifstream ifs(path, std::ios::in|std::ios::binary);
          PNG::File pngFile(ifs, {"tEXt"});
          for (const PNG::Chunk* chunk : pngFile.chunks()) {
            if (chunk->size() > 11 && memcmp(chunk->data(), "parameters", 10) == 0) {
              parameters = (chunk->data() + 11);
            }
          }
        }

        if (reqData.count("parameters") && reqData["parameters"].is_array()) {
          size_t rtnPos = parameters.find_first_of('\n');
          if (rtnPos != std::string::npos && rtnPos != 0) {
            // ignore Negative Params
            parameters = parameters.substr(0, rtnPos);
          }
          bool match = true;
          for (const auto& param : reqData["parameters"]) {
            if (parameters.find(param) == std::string::npos) {
              match = false;
              break;
            }
          }
          if (!match) continue;
        }

        files.push_back(path);
      }

      for (std::string& file : files) {
        if (file.size() <= library.size()) {
          std::cerr << "Path could not replaced: " << file << std::endl;
          continue;
        }
        file.replace(0, library.size() + 1, "");
      }

      HttpResponse(json(files)).send(destSock);
    }
    else if (parsedPath[1] == "library-list") {
      if (!m_config.is_object() || !m_config.count("media") || !(m_config["media"].is_object())) {
        HttpResponse response(404);
        response.send(destSock);
      }
      else {
        std::list<std::string> names;
        for (auto iter = m_config["media"].begin(); iter != m_config["media"].end(); ++iter) {
          names.push_back(iter.key());
        }
        HttpResponse(json(names)).send(destSock);
      }
    }
    else {
      HttpResponse(404).send(destSock);
    }
  }
  else {
    HttpResponse(404).send(destSock);
  }
}
