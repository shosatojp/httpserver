#include "common.hpp"
Client::Client(const char* ip_addr, int port)
    : ip_addr(ip_addr),
      port(port) {
    if ((sockfd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("socket");
    }
    client_addr.sin_family = PF_INET;
    client_addr.sin_addr.s_addr = ::inet_addr(ip_addr);
    client_addr.sin_port = ::htons(port);
}
Client::~Client() {
    if (sockfd)
        ::close(sockfd);
}
void Client::connect() {
    if (::connect(sockfd, (struct sockaddr*)&client_addr, sizeof(struct sockaddr)) < 0) {
        ::close(sockfd);
        throw std::runtime_error("connect");
    }
    std::cout << client_addr.sin_addr.s_addr << std::endl;
}
const char* Client::send(const char* src, size_t size) {
    if (::send(sockfd, src, size, 0) < 0) {
        ::close(sockfd);
        throw std::runtime_error("send");
    }
    std::cout << "=========" << std::endl;
    std::vector<char> buffer(static_cast<size_t>(1024));
    while (1) {
        constexpr size_t buf_size = 1024;
        char buf[buf_size] = {};
        std::cout << "before" << std::endl;
        size_t read_size = ::read(sockfd, buf, buf_size);  //::recv(sockfd, buf, buf_size, 0);
        std::cout << "after" << std::endl;

        std::cout << read_size << std::endl;
        if (read_size > 0) {
            std::cout << "read_size " << read_size << std::endl;
            std::cout << "buf " << buf << std::endl;
            buffer.insert(buffer.end(), buf, buf + read_size);
        } else {
            std::cout << "read_size(else)" << read_size << std::endl;
            break;
        }
    }
    std::cout << (const char*)&buffer.at(0) << std::endl;
}

void Client::close() {
    if (sockfd)
        ::close(sockfd);
}

int main() {
    Client client{"127.0.0.1", 8080};

    client.connect();
    client.send("GET HTTP1.1 /\nContent-Length:0", 4);
    client.close();
}