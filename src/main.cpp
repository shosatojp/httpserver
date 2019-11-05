#include "tinyhttp.hpp"

int main() {
    Router router{
        {
            {HttpMethod::GET, "/api", [](HttpRequest&& req, HttpResponse&& res) {
                 res.add_header("Content-Type", "application/json; charset=UTF-8");
                 res.add_body("{\"weather\":\"sunny\"}");
                 res(200);
             }},
            {HttpMethod::GET, "/", [](HttpRequest&& req, HttpResponse&& res) {
                 if (res.file(req.get_location().get_pathname())) {
                     res(200);
                 } else {
                     res(404);
                 }
             }},
        }};
    HttpServer{"127.0.0.1", 8080}
        .listen([router = std::move(router)](HttpRequest&& req, HttpResponse&& res) {
            router(std::move(req), std::move(res));
        });
}
