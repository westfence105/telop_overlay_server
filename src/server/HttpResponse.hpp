#pragma once

#include <fstream>
#include <unordered_map>

class HttpResponse {
  public:
    int status;
    std::unordered_map<std::string,std::string> headers;
    std::string body;

    HttpResponse(int status);
    HttpResponse(const std::string& str);
    HttpResponse(std::ifstream& file);

    void send(int destSock);
};
