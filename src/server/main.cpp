#include <iostream>
#include <string>
#include <stdexcept>

#include <HttpServer.hpp>

void usage(const std::string& appname);

int main(int argc, char** argv) {
  int port = 8080;

  if (argc > 1) {
    try {
      port = std::stoi(argv[1]);
    }
    catch (const std::invalid_argument& e) {
      usage(argv[0]);
      return -1;
    }
  }

  HttpServer server(port);
  server.start();

  return 0;
}

void usage(const std::string& appname) {
  std::cout << "Usage: " << appname << " [port]" << std::endl;
}
