# Minimal HTTP Server for C++

```cpp
int main() {
    Server{"127.0.0.1", 8080}.listen([](auto&& req, auto&& res) {
        res.add_header("Content-Type", "text/html; charset=UTF-8");
        res.add_body("<h1>ああ＾～こころがぴょんぴょんするんじゃぁ＾～～</h1>");
        res(200);
    });
}
```

# TODO
* バイト列送れるように(たぶんok)
* MimeTypeどうにかする
* いいかんじのログ
* keep_alive(ok)
* threading(ok)
* （非同期）
* ciやってみる