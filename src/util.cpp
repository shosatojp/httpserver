#include "util.hpp"
#include <algorithm>
#include <iomanip>
#include <string>

namespace util {
std::string trim(const std::string& s, const std::string& ws) noexcept {
    size_t i = s.find_first_not_of(ws);
    size_t j = s.find_last_not_of(ws);
    return i <= j ? s.substr(i, j - i + 1) : "";
}
bool starts_with(const std::string& s, const std::string& prefix) noexcept {
    return s.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), s.begin());
}

std::string url_decode(const std::string& s) {
    std::stringstream ss;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == '%') {
            if (s[i + 1] != '%') {
                unsigned int c;
                sscanf(s.substr(i + 1, 2).c_str(), "%x", &c);
                ss << static_cast<char>(c);
                i += 2;
            } else {
                ss << '%';
                i += 1;
            }
        } else {
            ss << s[i];
        }
    }
    return ss.str();
}

std::string url_encode(const std::string& s) {
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0, len = s.length(); i < len; i++) {
        char c = s[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            ss << c;
        } else {
            ss << '%' << std::uppercase << std::setw(2)
               << (int)(unsigned char)c
               << std::nouppercase;
        }
    }
    return ss.str();
}

}  // namespace util