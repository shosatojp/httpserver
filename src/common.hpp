#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <fstream>

class HttpMessage {
   public:
    HttpMessage() = default;
    void add_header(const std::string&& key, const std::string&& value);
    void add_header(const std::string&& key, const int value);

    inline void add_body(char* buf, size_t size);
    inline void add_body(char c);
    inline void add_body(const std::string&& str);
    void set_version(std::string& version);

   protected:
    std::map<std::string, std::string> headers;
    std::string version = "HTTP/1.1";
    std::vector<char> body;
    int header_count = -1;
    size_t body_size = 0;
};

void HttpMessage::add_body(char* buf, size_t size) {
    body.insert(body.end(), buf, buf + size);
    body_size += size;
}
void HttpMessage::add_body(char c) {
    body.push_back(c);
    body_size++;
}
void HttpMessage::add_body(const std::string&& str) {
    body.insert(body.end(), str.begin(), str.end());
    body_size += str.length();
}

class HttpRequest : public HttpMessage {
   public:
    HttpRequest(const struct sockaddr_in&& client);
    std::string text();
    std::string json();
    std::string to_string();
    using HttpMessage::add_header;
    long add_header(const std::string&& line);
    inline std::string get_method();
    inline std::string get_path();
    inline std::string get_addr();
    bool keep_alive();

   private:
    sockaddr_in addr;
    std::string path;
    std::string method;
};
std::string HttpRequest::get_method() {
    return method;
}
std::string HttpRequest::get_path() {
    return path;
}
std::string HttpRequest::get_addr() {
    return std::string(inet_ntoa(addr.sin_addr));
}

class HttpResponse : public HttpMessage {
   public:
    HttpResponse(int sockfd, bool keep_alive = false);
    void respond();
    void status(int status_code, const std::string& status_message = "");
    void operator()(int status_code = 200) {
        status(status_code);
        respond();
    }
    void file(const std::string& path);

   private:
    static const std::string root;
    int sockfd{};
    bool keep_alive;
    int status_code = 200;
    std::string status_message = "OK";
};

enum class HttpRead {
    Header,
    Body
};
class Server {
   public:
    Server(const char*, int);
    Server(Server&&) = default;
    Server& operator=(Server&&) = default;
    ~Server();

    void listen(const std::function<void(HttpRequest&&, HttpResponse&&)>&);
    void handle(const int sockfd, const struct sockaddr_in&& client, const std::function<void(HttpRequest&&, HttpResponse&&)>& handler);

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
