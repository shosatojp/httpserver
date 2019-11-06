#include "mimetypes.hpp"
#include "tinyhttp.hpp"
#include "util.hpp"

Location::Location(const std::string& src) {
    this->src = src;
    std::string search;
    // pathname
    auto pos = this->src.find('?');
    if (pos != std::string::npos) {
        this->pathname = util::url_decode(this->src.substr(0, pos));
        search = this->src.substr(pos + 1);
    } else {
        this->pathname = util::url_decode(this->src);
    }

    // query
    std::pair<std::string, std::string> pair;
    size_t prev = 0;
    for (size_t i = 0, count = search.length(); i < count; i++) {
        switch (search[i]) {
            case '=':
                pair.first = std::move(util::url_decode(search.substr(prev, i - prev)));
                prev = i + 1;
                break;
            case '&':
                pair.second = std::move(util::url_decode(search.substr(prev, i - prev)));
                if (pair.first.length() || pair.second.length())
                    this->query.insert(this->query.end(), std::move(pair));
                pair = std::pair<std::string, std::string>();
                prev = i + 1;
                break;
        }
    }
    pair.second = std::move(util::url_decode(search.substr(prev)));
    if (pair.first.length() || pair.second.length())
        this->query.insert(this->query.end(), std::move(pair));
}

std::string& Location::get_pathname() {
    return this->pathname;
}
std::unordered_map<std::string, std::string>& Location::get_query() {
    return this->query;
}

/**
 * HttpMethod
 */
const std::unordered_map<std::string, HttpMethod::_HttpMethod> HttpMethod::http_methods = {
    {"GET", HttpMethod::GET},
    {"POST", HttpMethod::POST},
    {"PUT", HttpMethod::PUT},
    {"DELETE", HttpMethod::DELETE},
    {"PATCH", HttpMethod::PATCH},
};

HttpMethod::_HttpMethod HttpMethod::from_string(const std::string& s) noexcept {
    auto it = http_methods.find(s);
    return it != http_methods.end() ? it->second : _HttpMethod::GET;
}

std::string HttpMethod::to_string(const _HttpMethod m) noexcept {
    auto it = std::find_if(http_methods.begin(), http_methods.end(), [m](const std::pair<std::string, _HttpMethod>& pair) {
        return m == pair.second;
    });
    return it != http_methods.end() ? it->first : "<UNSUPPORTED METHOD>";
}

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
        std::string _method;
        iss >> _method >> path >> version;
        std::transform(_method.cbegin(), _method.cend(), _method.begin(), toupper);
        this->method = HttpMethod::from_string(_method);
        this->location = Location(this->path);
        header_count = 0;
    } else {
        // real header
        size_t sep = line.find(':');
        std::string key = util::trim(line.substr(0, sep)),
                    value = util::trim(line.substr(sep + 1));
        // normarize
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
Location& HttpRequest::get_location() {
    return this->location;
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

const std::string HttpResponse::root = (std::string)std::filesystem::current_path() + std::string("/");

bool HttpResponse::file(const std::string& raw_path) {
    if (raw_path.size() > 0) {
        std::string&& path = raw_path.substr(1);
        if (path.length() == 0) path = ".";
        std::filesystem::path absolute_path = std::filesystem::absolute(path).lexically_normal();
        if (std::filesystem::is_directory(absolute_path)) {
            absolute_path /= "index.html";
        }
        this->add_header("Content-Type", get_mimetype(std::filesystem::path(absolute_path).extension()));
        if (util::starts_with(absolute_path, root) && std::filesystem::exists(absolute_path)) {
            std::ifstream ifs{absolute_path, std::ios::binary | std::ios::in};
            ifs >> std::noskipws;
            std::copy(std::istream_iterator<char>(ifs),
                      std::istream_iterator<char>(),
                      std::back_inserter(body));
            return true;
        }
    }
    return false;
}