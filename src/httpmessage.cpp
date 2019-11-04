#include "common.hpp"

/**
 * HttpMessage
*/
void HttpMessage::add_header(const std::string&& key, const std::string&& value) {
    headers.insert(std::make_pair(key, value));
    header_count++;
}
void HttpMessage::add_header(const std::string&& key, const int value) {
    add_header(std::move(key), std::move(std::to_string(value)));
}
void HttpMessage::set_version(std::string& version) {
    this->version = version;
}

/**
 * HttpRequest
 */
HttpRequest::HttpRequest(const struct sockaddr_in&& client) : addr(client) {}

long HttpRequest::add_header(const std::string&& line) {
    size_t content_length = -1;
    if (header_count == -1) {
        // versions
        std::istringstream iss{line};
        iss >> method >> path >> version;
        std::transform(method.cbegin(), method.cend(), method.begin(), toupper);
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

std::string HttpRequest::text() {
    return std::string(body.data());
}

std::string HttpRequest::to_string() {
    std::stringstream ss;
    ss << method << " " << path << " " << version << std::endl;
    for (auto&& [key, value] : headers) {
        ss << key << " : " << value << std::endl;
    }
    ss << "<body(size = " << body.size() << ")>";
    return ss.str();
}

bool HttpRequest::keep_alive() {
    std::string key{"CONNECTION"};
    std::string& value = headers[key];
    std::transform(value.cbegin(), value.cend(), value.begin(), toupper);
    return headers.count(key) && headers[key] == "KEEP-ALIVE";
}

/**
 * HttpResponse
 */
void HttpResponse::respond() {
    add_header("Content-Length", this->body.size());
    std::stringstream ss;
    ss << this->version << " " << this->status_code << " " << this->status_message << "\r\n";
    for (auto&& [key, value] : this->headers)
        ss << key << ": " << value << "\r\n";
    ss << "\r\n";
    std::string&& header = ss.str();

    std::vector<char> b{};
    b.insert(b.end(), header.begin(), header.end());
    b.insert(b.end(), this->body.begin(), this->body.end());

    if (::write(this->sockfd, b.data(), b.size()) != b.size()) {
        ::perror("write");
        throw std::runtime_error("error at writing socket.");
    }
    if (!keep_alive) ::close(this->sockfd);
    return;
}

HttpResponse::HttpResponse(int sockfd, bool keep_alive) : sockfd(sockfd), keep_alive(keep_alive) {
}

void HttpResponse::status(int status_code, const std::string& status_message) {
    this->status_code = status_code;
    this->status_message = status_message;
}

bool starts_with(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), s.begin());
}

const std::string HttpResponse::root = (std::string)std::filesystem::current_path() + std::string("/");

bool HttpResponse::file(const std::string& raw_path) {
    if (raw_path.size() > 0) {
        std::string&& path = raw_path.substr(1);
        if (path.length() > 0) {
            std::string absolute_path = std::filesystem::absolute(path).lexically_normal();
            if (starts_with(absolute_path, root) && std::filesystem::exists(absolute_path)) {
                std::ifstream ifs{absolute_path, std::ios::binary | std::ios::in};
                ifs >> std::noskipws;
                std::copy(std::istream_iterator<char>(ifs),
                          std::istream_iterator<char>(),
                          std::back_inserter(body));
                return true;
            }
        }
    }
    return false;
}