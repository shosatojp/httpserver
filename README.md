# Minimal HTTP HttpServer for C++

```cpp
int main() {
    HttpServer{"127.0.0.1", 8080}.listen([](auto&& req, auto&& res) {
        res.add_header("Content-Type", "text/html; charset=UTF-8");
        res.add_body("<h1>ああ＾～こころがぴょんぴょんするんじゃぁ＾～～</h1>");
        res(200);
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