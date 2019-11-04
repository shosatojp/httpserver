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
    {".ico", "image/x-icon"},
};

std::string get_mimetype(const std::string& ext);