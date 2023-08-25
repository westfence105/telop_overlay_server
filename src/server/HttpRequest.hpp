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

    inline HttpRequestType    type() const { return m_type; }
    inline const std::string& path() const { return m_path; }
    inline const std::string& body() const { return m_body; }
    inline const std::string& header(const std::string& key) const {
      auto iter = m_headers.find(key);
      if (iter != m_headers.cend()) {
        return iter->second;
      }
      else {
        static const std::string empty = "";
        return empty;
      }
    }
};
