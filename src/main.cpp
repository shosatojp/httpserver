#include "common.hpp"
#include "mimetypes.hpp"

int main() {
    Router router{
        {
            {"/api", [](HttpRequest&& req, HttpResponse&& res) {
                 res.add_header("Content-Type", "application/json; charset=UTF-8");
                 res.add_body("{\"hoge\":\"hage\"}");
                 res(200);
             }},
            {"/", [](HttpRequest&& req, HttpResponse&& res) {
                 res.add_header("Content-Type", get_mimetype(std::filesystem::path(req.get_path()).extension()));
                 if (res.file(req.get_path())) {
                     res(200);
                 } else {
                     res(404);
                 }
             }},
        }};
    Server{"127.0.0.1", 8081}
        .listen([router = std::move(router)](HttpRequest&& req, HttpResponse&& res) {
            router(std::move(req), std::move(res));
        });
}
