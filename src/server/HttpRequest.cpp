#include <iostream>
#include <sstream>

#include <HttpRequest.hpp>

HttpRequest::HttpRequest(const std::string& requestStr) {
  std::istringstream iss(requestStr);

  std::string typeStr;
  iss >> typeStr;
  if (typeStr == "GET") {
    m_type = HttpRequestType::GET;
  }
  else if (typeStr == "POST") {
    m_type = HttpRequestType::POST;
  }

  iss >> m_path;
  if (m_path == "") {
    m_path = "index.html";
  }

  std::string line;
  std::getline(iss, line);
  while (std::getline(iss, line)) {
    if (line.size() > 0 && line[line.size()-1] == '\n') line.erase(line.size()-1);
    if (line.size() > 0 && line[line.size()-1] == '\r') line.erase(line.size()-1);
    if (line.size() == 0) {
      break;
    }
    size_t delimPos = line.find_first_of(':');
    if (delimPos != std::string::npos) {
      std::string key = line.substr(0, delimPos);
      m_headers[key] = (delimPos + 2 < line.size()) ? line.substr(delimPos + 2) : "";
    }
  }

  std::istreambuf_iterator<char> begin(iss);
  std::istreambuf_iterator<char> end;
  m_body = std::string(begin, end);
}
