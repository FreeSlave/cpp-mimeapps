#ifndef MIMEAPPS_PATH_H
#define MIMEAPPS_PATH_H

#include <string>

namespace mimeapps
{
    std::string buildPath(const std::string& path, const std::string& append);

    template<typename Iterator>
    bool isBaseName(Iterator first, Iterator last) {
        if (first == last) {
            return false;
        }
        for (Iterator it = first; it != last; ++it) {
            if (*it == '/') {
                return false;
            }
        }
        return true;
    }

    bool isBaseName(const std::string& path);
    bool isBaseName(const char* path);

    template<typename Iterator>
    bool isAbsolutePath(Iterator first, Iterator last) {
        return (first != last && *first == '/');
    }

    bool isAbsolutePath(const std::string& path);
    bool isAbsolutePath(const char* path);
}

#endif
