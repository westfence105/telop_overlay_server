#include <iostream>
#include <sstream>
#include <vector>
#include <functional>

#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#include <HttpServer.hpp>
#include <HttpRequest.hpp>
#include <HttpResponse.hpp>

using namespace std::literals::string_literals;

HttpServer::HttpServer(int port) {
  m_port = port;
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
  timeout.tv_sec = 5;
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
  m_threads.push_back(std::thread([&] () {
    char buf;
    std::cout << "Server running on port " << m_port << "...\nPress 'x' to stop." << std::endl;
    do {
      std::cin >> buf;
    } while (buf != 'x');
    loop = false;
  }));

  listen(srcSock, 10);
  while (loop) {
    destSock = accept(srcSock, (struct sockaddr*)&destAddr, &destAddrSize);
    if (destSock >= 0) {
      m_threads.push_back(std::thread(std::bind(&HttpServer::handle_request, this, destSock)));
    }
  }

  for (auto& t : m_threads) {
    t.join();
  }

  close(srcSock);
}

void HttpServer::handle_request(int destSock) {
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
  if (path.starts_with("/api")) {
    std::vector<std::string> parsedPath;
    size_t offset = 5;
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
    if (parsedPath.size() < 1) {
      HttpResponse response(404);
      response.send(destSock);
    }
    else if (parsedPath.size() == 2 && parsedPath[0] == "variable") {
      std::unique_lock lock(m_mutex);
      if (request.type() == HttpRequestType::POST) {
        m_variables[parsedPath[1]] = request.body();
      }
      HttpResponse response(m_variables[parsedPath[1]]);
      response.send(destSock);
    }
    else {
      HttpResponse response(404);
      response.send(destSock);
    }
  }
  else {
    std::string ext = "html";
    size_t extPos = path.find_first_of('.');
    if (extPos == std::string::npos) {
      path = path + ".html";
    }
    else if (extPos+1 < path.length()) {
      ext = path.substr(extPos+1);
    }
    std::ifstream file("pages/"s + path);
    if (file) {
      HttpResponse response(file);
      if (ext == "html") {
        response.headers["Content-Type"] = "text/html";
      }
      else if (ext == "js") {
        response.headers["Content-Type"] = "text/javascript";
      }
      else if (ext == "css") {
        response.headers["Content-Type"] = "text/css";
      }
      else {
        response.headers["Content-Type"] = "text/plain";
      }
      response.send(destSock);
    }
    else {
      HttpResponse response(404);
      response.send(destSock);
    }
  }

  close(destSock);
}
