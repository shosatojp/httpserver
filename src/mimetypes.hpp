#include <iostream>
#include <string>
#include <unordered_map>

const std::unordered_map<std::string, std::string> mimetypes = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "text/javascript"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".json", "application/json"},
};

std::string get_mimetype(const std::string& ext) {
    auto itr = mimetypes.find(ext);
    if (itr != mimetypes.end()) {
        return itr->second;
    } else {
        return "text/plain";
    }
}