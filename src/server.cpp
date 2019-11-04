
#include "common.hpp"

Server::Server(const char* bind_ip, int port) : bind_ip(bind_ip), port(port) {
    reader_addr.sin_family = PF_INET;
    reader_addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    reader_addr.sin_port = ::htons(port);

    signal(SIGPIPE, NULL);
}

[[noreturn]] void Server::listen(const HttpHandler& handler) {
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
            continue;
        }

        std::thread{[this, new_sockfd = std::move(new_sockfd), writer_addr = std::move(writer_addr), handler = std::move(handler)] {
            try {
                handle(new_sockfd, std::move(writer_addr), handler);
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }}
            .detach();
    }
}

void Server::handle(const int sockfd, const struct sockaddr_in&& client, const HttpHandler& handler) {
    while (1) {  // loop for keep alive
        HttpRead status = HttpRead::Header;
        std::stringstream token;
        size_t read_size = 0;
        size_t content_length = 0;
        size_t body_begin = 0;
        HttpRequest req{std::move(client)};
        while (1) {
            size_t rsize;
            char buf[100] = {};
            if ((rsize = ::read(sockfd, buf, sizeof(buf))) <= 0)
                break;
            read_size += rsize;
            switch (status) {
                case HttpRead::Header: {
                    for (size_t i = 0; i < rsize; i++) {
                        char c = buf[i];
                        if (c == '\n') {
                            token.seekg(0, std::ios::end);
                            if (token.tellg() == 0) {  // End of Header
                                body_begin = read_size - rsize + i;
                                status = HttpRead::Body;
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
                                case HttpRead::Header:
                                    token << c;
                                    break;
                                case HttpRead::Body:
                                    req.add_body(c);
                                    break;
                            }
                        }
                    }
                } break;
                case HttpRead::Body:
                    token << buf;
                    req.add_body(buf, sizeof buf);
                    break;
            }
            if (read_size == body_begin + 1 + content_length) {
                break;
            }
        }

        // log
        bool keep_alive = req.keep_alive();
        std::cout << req.get_addr() << " " << req.get_method() << " " << req.get_path() << std::endl;
        HttpResponse res{sockfd, keep_alive};
        handler(std::move(req), std::move(res));

        if (!keep_alive) break;
    }
}

Server::~Server() noexcept {
    if (this->sockfd)
        ::close(this->sockfd);
}
