#pragma once

#include <string>
#include <unordered_map>

enum HttpRequestType {
  GET, POST,
};

class HttpRequest {
  HttpRequestType m_type;
  std::string m_path;
  std::unordered_map<std::string,std::string> m_headers;
  std::string m_body;

  public:
    HttpRequest(const std::string& requestStr);

    inline HttpRequestType type() { return m_type; }
    inline const std::string& path() { return m_path; }
    inline const std::string& header(const std::string& key) { return m_headers[key]; }
    inline const std::string& body() { return m_body; }
};
