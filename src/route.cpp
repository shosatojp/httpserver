#include "header.hpp"
#include "mimetypes.hpp"
#include "util.hpp"

Router::Router(const std::vector<std::pair<const std::string, const HttpHandler>>& table):table(table) {
}

bool Router::operator()(HttpRequest&& req, HttpResponse&& res) const {
    std::string&& path = req.get_path();
    for (auto&& [prefix, handler] : this->table) {
        if (util::starts_with(path, prefix)) {
            handler(std::move(req), std::move(res));
            return true;
        }
    }
    return false;
}
