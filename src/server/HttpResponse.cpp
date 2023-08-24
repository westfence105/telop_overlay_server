#include <iostream>
#include <sstream>

#include <unistd.h>
#include <sys/socket.h>

#include <HttpResponse.hpp>

HttpResponse::HttpResponse(int status) {
  this->status = status;
  this->headers["Connection"] = "Close";
}

HttpResponse::HttpResponse(const std::string& str) {
  this->status = 200;
  this->body = str;
  this->headers["Content-Type"] = "text/plain";
  this->headers["Connection"] = "Close";
}

HttpResponse::HttpResponse(std::ifstream& file) {
  this->status = 200;

  std::istreambuf_iterator<char> begin(file);
  std::istreambuf_iterator<char> end;
  this->body = std::string(begin, end);

  this->headers["Connection"] = "Close";
}

void HttpResponse::send(int destSock) {
  std::ostringstream oss;
  oss << "HTTP/1.1 " << status << " " << (status == 200 ? "OK" : "BAD") << "\r\n";
  for (auto e : headers) {
    oss << e.first << ": " << e.second << "\r\n";
  }
  oss << "\r\n" << body;

  ::send(destSock, oss.str().c_str(), oss.str().length(), 0);
}
