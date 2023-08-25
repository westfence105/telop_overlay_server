#pragma once

#include <fstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

class HttpResponse {
  public:
    int status;
    std::unordered_map<std::string,std::string> headers;
    std::string body;

    HttpResponse(int status);
    HttpResponse(const std::string& str);
    HttpResponse(const nlohmann::json& json);
    HttpResponse(std::ifstream& file, const std::string& ext);

    void send(int destSock);
};
