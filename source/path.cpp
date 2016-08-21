#include "path.h"
#include <cstring>

namespace mimeapps
{
    std::string buildPath(const std::string& path, const std::string& append)
    {
        if (isAbsolutePath(append)) {
            return append;
        } else if (append.empty()) {
            return path;
        } else if (path.empty()) {
            return append;
        } else {
            if (path[path.size()-1] == '/') {
                return path + append;
            } else {
                return path + '/' + append;
            }
        }
    }

    bool isBaseName(const std::string& path) {
        return isBaseName(path.begin(), path.end());
    }

    bool isBaseName(const char* path) {
        if (!path || !*path) {
            return false;
        }
        for(; *path; ++path) {
            if (*path == '/') {
                return false;
            }
        }
        return true;
    }

    bool isAbsolutePath(const std::string& path)
    {
        return isAbsolutePath(path.begin(), path.end());
    }

    bool isAbsolutePath(const char* path)
    {
        return path && *path == '/';
    }
}
