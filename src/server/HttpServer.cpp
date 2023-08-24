#include <iostream>
#include <sstream>
#include <vector>

#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <HttpServer.hpp>
#include <HttpRequest.hpp>
#include <HttpResponse.hpp>

using namespace std::literals::string_literals;

HttpServer::HttpServer(int port) {
  m_port = port;
}

void HttpServer::start() {
  int srcSock, destSock;
  struct sockaddr_in srcAddr, destAddr;
  socklen_t destAddrSize;
  size_t recvSize;
  char buf[BUFSIZE+1];

  srcAddr.sin_port = htons(m_port);
  srcAddr.sin_family = AF_INET;
  srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  srcSock = socket(AF_INET, SOCK_STREAM, 0);
  bind(srcSock, (struct sockaddr*)&srcAddr, sizeof(srcAddr));

  while(1) {
    std::ostringstream oss;
    listen(srcSock, 3);
    destSock = accept(srcSock, (struct sockaddr*)&destAddr, &destAddrSize);

    do {
      memset(buf, 0, BUFSIZE+1);
      recvSize = recv(destSock, buf, BUFSIZE, 0);
      oss << buf;
    } while (recvSize >= BUFSIZE);

    handle_request(oss.str(), destSock);

    close(destSock);
  }
}

void HttpServer::handle_request(const std::string& requestStr, int destSock) {
  HttpRequest request(requestStr);

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
}
