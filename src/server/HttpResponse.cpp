#include <iostream>
#include <sstream>
#include <cstring>

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
  this->headers["Cache-Control"] = "max-age=600";
  this->headers["Connection"] = "Close";
}

HttpResponse::HttpResponse(const nlohmann::json& json) {
  this->status = 200;
  this->body = json.dump();
  this->headers["Content-Type"] = "application/json";
  this->headers["Cache-Control"] = "max-age=600";
  this->headers["Connection"] = "Close";
}

static constexpr unsigned char MAGIC_JPG[] = {0xFF, 0xD8};
static constexpr unsigned char MAGIC_PNG[] = {0x89, 'P', 'N', 'G'};
static constexpr unsigned char MAGIC_MP4[] = {0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70, 0x69, 0x73, 0x6F, 0x6D, 0x00, 0x00, 0x02, 0x00};

HttpResponse::HttpResponse(std::ifstream& file, const std::string& ext) {
  this->status = 200;

  std::istreambuf_iterator<char> begin(file);
  std::istreambuf_iterator<char> end;
  this->body = std::string(begin, end);

  if (ext == "html") {
    this->headers["Content-Type"] = "text/html";
  }
  else if (ext == "js") {
    this->headers["Content-Type"] = "text/javascript";
  }
  else if (ext == "css") {
    this->headers["Content-Type"] = "text/css";
  }
  else if (ext == "jpg" || ext == "jpeg" || (body.size() >= sizeof(MAGIC_JPG) && memcmp(body.c_str(), MAGIC_JPG, sizeof(MAGIC_JPG)) == 0)) {
    this->headers["Content-Type"] = "image/jpeg";
  }
  else if (ext == "png" || (body.size() >= sizeof(MAGIC_PNG) && memcmp(body.c_str(), MAGIC_PNG, sizeof(MAGIC_PNG)) == 0)) {
    this->headers["Content-Type"] = "image/png";
  }
  else if (ext == "gif" || (body.size() >= 3 && memcmp(body.c_str(), "GIF", 3) == 0)) {
    this->headers["Content-Type"] = "image/png";
  }
  else if (ext == "bmp" || (body.size() >= 2 && memcmp(body.c_str(), "BM", 2) == 0)) {
    this->headers["Content-Type"] = "image/bmp";
  }
  else if (ext == "mp3" || (body.size() >= 3 && memcmp(body.c_str(), "ID3", 3) == 0)) {
    this->headers["Content-Type"] = "audio/mpeg";
  }
  else if (ext == "m4a") {
    this->headers["Content-Type"] = "audio/aac";
  }
  else if (ext == "mp4" || (body.size() >= sizeof(MAGIC_MP4) && memcmp(body.c_str(), MAGIC_MP4, sizeof(MAGIC_MP4)) == 0)) {
    this->headers["Content-Type"] = "video/mp4";
  }
  else if (ext == "pdf" || memcmp(body.c_str(), "%PDF", 4) == 0) {
    this->headers["Content-Type"] = "application/pdf";
  }
  else if (file.binary) {
    this->headers["Content-Type"] = "application/octet-stream";
  }
  else {
    this->headers["Content-Type"] = "text/plain";
  }

  this->headers["Cache-Control"] = "max-age=604800";
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
