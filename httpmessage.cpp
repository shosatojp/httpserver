#include "common.hpp"

inline void HttpMessage::add_header(const std::string&& key, const std::string&& value) {
    headers.insert(std::make_pair(key, value));
    header_count++;
}

long HttpRequest::add_header(const std::string&& line) {
    size_t content_length = -1;
    if (header_count == -1) {
        // versions
        std::istringstream iss{line};
        iss >> method >> path >> version;
        std::transform(method.cbegin(), method.cend(), method.begin(), toupper);
        // std::cout << method << "    " << version << "   " << path << std::endl;
        header_count = 0;
    } else {
        // real header
        size_t sep = line.find(':');
        std::string key = line.substr(0, sep),
                    value = line.substr(sep + 1);
        // normarize
        boost::trim(key);
        boost::trim(value);
        std::transform(key.cbegin(), key.cend(), key.begin(), toupper);

        if (key == "CONTENT-LENGTH") {
            content_length = std::stoi(value);
        }
        add_header(std::move(key), std::move(value));
    }
    return content_length;
}

void HttpMessage::add_header(const std::string&& key, const int value) {
    add_header(std::move(key), std::to_string(value));
}

std::string HttpRequest::text() {
    return body.str();
}

std::string HttpRequest::to_string() {
    std::stringstream ss;
    ss << method << " " << path << " " << version << std::endl;
    for (auto&& [key, value] : headers) {
        ss << key << " : " << value << std::endl;
    }
    ss << "<body(size = " << body_size << ")>";
    return ss.str();
}

void HttpResponse::response() {
    add_header("Content-Length", body_size);
    std::stringstream ss;
    ss << version << " " << status_code << " " << status_message << "\r\n";
    for (auto&& [key, value] : headers) {
        ss << key << ": " << value << "\r\n";
    }
    ss << "\r\n";
    ss << body.str().c_str();

    std::string msg = ss.str();
    int s;
    if ((s = ::write(this->sockfd, msg.c_str(), msg.length())) != msg.length()) {
        ::perror("write");
        throw std::runtime_error("error at writing socket.");
    }
    std::cout << s << std::endl;
    ::close(this->sockfd);
    return;
}

HttpResponse::HttpResponse(int sockfd) : sockfd(sockfd) {
}

void HttpResponse::status(int status_code, const std::string& status_message) {
    this->status_code = status_code;
    this->status_message = status_message;
}