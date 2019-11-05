# Tiny HTTP HttpServer for C++
## Minimal
```cpp
#include "header.hpp"

int main() {
    // Start http server
    HttpServer{"127.0.0.1", 8080}
        .listen([router = std::move(router)](HttpRequest&& req, HttpResponse&& res) {
            // do something
        });
}
```


## Routing
```cpp
#include "header.hpp"

int main() {

    // Routing definition
    Router router{
        {
            // Route for some api
            {HttpMethod::GET, "/api", [](HttpRequest&& req, HttpResponse&& res) {

                 // Set Header and Body
                 res.add_header("Content-Type", "application/json; charset=UTF-8");
                 res.add_body("{\"weather\":\"sunny\"}");

                 // Respond with status code.
                 res(200); 
             }},

            // Route for files (fallback). Matches both GET and POST.
            {HttpMethod::GET | HttpMethod::POST, "/", [](HttpRequest&& req, HttpResponse&& res) {

                // Load file content and automatically solve mimetype.
                 if (res.file(req.get_location().get_pathname())) {
                     res(200);
                 } else {
                     // File not found
                     res(404);
                 }
             }},
        }};

    // Start http server
    HttpServer{"127.0.0.1", 8080}
        .listen([router = std::move(router)](HttpRequest&& req, HttpResponse&& res) {
            router(std::move(req), std::move(res));
        });
}
```

# TODO
* バイト列送れるように(たぶんok)
* MimeTypeどうにかする(ok)
* いいかんじのログ(ok)
* keep_alive(ok)
* threading(ok)
* （非同期）
* ciやってみる
* index.html(ok)
* %encoding