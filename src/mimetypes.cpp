#include "mimetypes.hpp"



std::string get_mimetype(const std::string& ext) {
    auto itr = mimetypes.find(ext);
    if (itr != mimetypes.end()) {
        return itr->second;
    } else {
        return "text/plain";
    }
}