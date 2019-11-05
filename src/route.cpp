#include "header.hpp"
#include "mimetypes.hpp"
#include "util.hpp"

Router::Router(const std::vector<std::tuple<const HttpMethod::_HttpMethod, const std::string, const HttpHandler>>& table) : table(table) {
}

bool Router::operator()(HttpRequest&& req, HttpResponse&& res) const {
    std::string&& path = req.get_path();
    for (auto&& [method, prefix, handler] : this->table) {
        if ((method & req.get_method()) && util::starts_with(path, prefix)) {
            handler(std::move(req), std::move(res));
            return true;
        }
    }
    res.status(400);
    res.respond();
    return false;
}
