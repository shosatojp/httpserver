#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class HttpMessage {
   public:
    HttpMessage() = default;
    void add_header(const std::string&& key, const std::string&& value);
    void add_header(const std::string&& key, const int value);

    inline void add_body(char* buf, size_t size) {
        body.sputn(buf, size);
        body_size += size;
    }
    inline void add_body(char c) {
        body.sputc(c);
        body_size++;
    }
    inline void add_body(const std::string&& str) {
        body.sputn(str.c_str(), str.length());
        body_size += str.length();
    }
    void set_version(std::string& version) {
        this->version = version;
    }

   protected:
    std::map<std::string, std::string> headers;
    std::string version = "HTTP/1.1";
    std::stringbuf body;
    int header_count = -1;
    size_t body_size = 0;
};

class HttpRequest : public HttpMessage {
   public:
    using HttpMessage::HttpMessage;
    std::string text();
    std::string json();
    std::string to_string();
    using HttpMessage::add_header;
    long add_header(const std::string&& line);

   private:
    std::string path;
    std::string method;
};

class HttpResponse : public HttpMessage {
   public:
    HttpResponse(int sockfd);
    void response();
    void status(int status_code, const std::string& status_message="");

   private:
    int sockfd{};
    int status_code = 200;
    std::string status_message = "OK";
};

enum class HTTPRead {
    Header,
    Body
};
class Server {
   public:
    Server(const char*, int);
    Server(Server&&) = default;
    Server& operator=(Server&&) = default;
    ~Server();

    void listen(std::function<void(HttpRequest&&, HttpResponse&&)>);
    void handle(int sockfd, std::function<void(HttpRequest&&, HttpResponse&&)> handler);

   private:
    Server() = delete;
    Server& operator=(const Server&) = delete;
    Server(const Server&) = delete;
    int sockfd{};
    sockaddr_in reader_addr{};
    const char* bind_ip{};
    int port{};
};

class Client {
   public:
    Client(const char*, int);
    Client(Client&&) = default;
    Client& operator=(Client&&) = default;
    ~Client();

    void connect();
    const char* send(const char*, size_t);
    void close();

   private:
    Client& operator=(const Client&) = delete;
    Client(const Client&) = delete;
    int sockfd{};
    sockaddr_in client_addr{};
    const char* ip_addr{};
    int port{};
};