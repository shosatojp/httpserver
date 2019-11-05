#include <string>

namespace util {
std::string trim(const std::string& s, const std::string& ws = " \n\r\t") noexcept;
bool starts_with(const std::string& s, const std::string& prefix) noexcept;
}