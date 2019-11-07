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
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#if __GNUC__ >= 9
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std {
namespace filesystem = std::experimental::filesystem;
}
#endif

class Location {
   public:
    Location() = default;
    Location(const std::string& src);
    std::string& get_pathname();
    std::unordered_map<std::string, std::string>& get_query();

   private:
    std::unordered_map<std::string, std::string> query;
    std::string pathname;
    std::string src;
};

class HttpMethod {
   public:
    enum _HttpMethod : char {
        GET = 1,
        POST = 2,
        PUT = 4,
        DELETE = 8,
        PATCH = 16
    };
    static _HttpMethod from_string(const std::string& s) noexcept;
    static std::string to_string(const _HttpMethod m) noexcept;

   private:
    HttpMethod() = delete;
    static const std::unordered_map<std::string, _HttpMethod> http_methods;
};

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
};

void HttpMessage::add_body(char* buf, size_t size) {
    body.insert(body.end(), buf, buf + size);
}
void HttpMessage::add_body(char c) {
    body.push_back(c);
}
void HttpMessage::add_body(const std::string&& str) {
    body.insert(body.end(), str.begin(), str.end());
}

class HttpRequest : public HttpMessage {
   public:
    HttpRequest(const struct sockaddr_in&& client);
    std::string text();
    std::string json();
    std::string to_string();
    using HttpMessage::add_header;
    long add_header(const std::string&& line);
    inline HttpMethod::_HttpMethod get_method();
    inline std::string get_path();
    inline std::string get_addr();
    Location& get_location();
    bool keep_alive();

   private:
    Location location;
    sockaddr_in addr;
    std::string path;
    HttpMethod::_HttpMethod method;
};
HttpMethod::_HttpMethod HttpRequest::get_method() {
    return this->method;
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
    bool file(const std::string& path);

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

using HttpHandler = std::function<void(HttpRequest&&, HttpResponse&&)>;

class HttpServer {
   public:
    HttpServer(const char*, int);
    HttpServer(HttpServer&&) = default;
    HttpServer& operator=(HttpServer&&) = default;
    ~HttpServer();

    void listen(const HttpHandler&);
    void handle(const int sockfd, const struct sockaddr_in&& client, const HttpHandler& handler);

   private:
    HttpServer() = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    HttpServer(const HttpServer&) = delete;
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

class Router {
   public:
    Router(const std::vector<std::tuple<const HttpMethod::_HttpMethod, const std::string, const HttpHandler>>& table);
    bool operator()(HttpRequest&& req, HttpResponse&& res) const;

   private:
    std::vector<std::tuple<const HttpMethod::_HttpMethod, const std::string, const HttpHandler>> table;
};
