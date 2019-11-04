
#include "common.hpp"

Server::Server(const char* bind_ip = "127.0.0.1", int port = 8080) : bind_ip(bind_ip), port(port) {
    reader_addr.sin_family = PF_INET;
    reader_addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    reader_addr.sin_port = ::htons(port);
}

[[noreturn]] void Server::listen(std::function<void(HttpRequest&&, HttpResponse&&)> handler) {
    if ((sockfd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("socket");
    }

    if (::bind(sockfd, (struct sockaddr*)&reader_addr, sizeof reader_addr) < 0) {
        throw std::runtime_error("bind");
    }

    if (::listen(sockfd, SOMAXCONN) < 0) {
        ::close(sockfd);
        throw std::runtime_error("listen");
    }
    while (1) {
        int new_sockfd;
        sockaddr_in writer_addr{};
        int writer_len = sizeof writer_addr;
        if ((new_sockfd = ::accept(sockfd, (struct sockaddr*)&writer_addr, (socklen_t*)&writer_len)) < 0) {
            ::close(sockfd);
            throw std::runtime_error("accept");
        }
        std::cout << "client accepted " << new_sockfd << std::endl;
        handle(new_sockfd, handler);
    }
}

void Server::handle(int sockfd, std::function<void(HttpRequest&&, HttpResponse&&)> handler) {
    HTTPRead status = HTTPRead::Header;
    std::stringstream token;
    size_t read_size = 0;
    size_t content_length = 0;
    size_t body_begin = 0;
    HttpRequest req{};
    while (1) {
        size_t rsize;
        char buf[100] = {};
        if ((rsize = ::read(sockfd, buf, sizeof(buf))) <= 0)
            break;
        read_size += rsize;
        switch (status) {
            case HTTPRead::Header: {
                for (size_t i = 0; i < rsize; i++) {
                    char c = buf[i];
                    if (c == '\n') {
                        token.seekg(0, std::ios::end);
                        if (token.tellg() == 0) {  // End of Header
                            body_begin = read_size - rsize + i;
                            status = HTTPRead::Body;
                        } else {
                            long _len = 0;
                            if ((_len = req.add_header(std::move(token.str()))) > -1) {
                                content_length = _len;
                            }
                        }
                        token = std::stringstream();
                    } else if (c == '\r') {
                    } else {
                        switch (status) {
                            case HTTPRead::Header:
                                token << c;
                                break;
                            case HTTPRead::Body:
                                req.add_body(c);
                                break;
                        }
                    }
                }
            } break;
            case HTTPRead::Body:
                token << buf;
                req.add_body(buf, sizeof buf);
                break;
        }
        // std::cout << "read_size " << read_size << std::endl;
        // std::cout << "body_begin " << body_begin << std::endl;
        // std::cout << "content_length " << content_length << std::endl;
        if (read_size == body_begin + 1 + content_length) {
            break;
        }
    }
    // std::cout << req.to_string() << std::endl;
    // std::cout << req.text() << std::endl;
    // const char* msg = "HTTP/1.1 200 OK\r\n";
    // std::cout << ::write(sockfd, msg, strlen(msg)) << std::endl;
    HttpResponse res{sockfd};
    handler(std::move(req), std::move(res));
    // ::close(sockfd);
}

Server::~Server() noexcept {
    if (this->sockfd)
        ::close(this->sockfd);
}

int main() {
    Server server{"127.0.0.1", 8080};
    server.listen([](HttpRequest&& req, HttpResponse&& res) {
        // res.add_body('h');

        res.add_header("hoge", "hige");
        res.add_body("hogehoge");

        res.status(418);
        res.response();
    });
}
