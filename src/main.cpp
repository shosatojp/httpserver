#include "common.hpp"

int main() {
    Server{"127.0.0.1", 8080}.listen([](HttpRequest&& req, HttpResponse&& res) {
        res.add_header("Content-Type", "text/html; charset=UTF-8");
        // res.add_body("<h1>ああ＾～こころがぴょんぴょんするんじゃぁ＾～～</h1>");
        std::string&& path = req.get_path();
        if (path.size() > 0 && res.file(path.substr(1))) {
            res(200);
        } else {
            res(404);
        }
    });
}
