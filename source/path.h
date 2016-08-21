// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

/**
 * \file
 * \brief Various functions to work with paths.
 */

#ifndef MIMEAPPS_PATH_H
#define MIMEAPPS_PATH_H

#include <string>

namespace mimeapps
{
    
    /// \brief Concat two paths.
    std::string buildPath(const std::string& path, const std::string& append);

    
    /// \brief Check if path is base name, i.e. not absolute, nor relative with dots.
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

    ///ditto
    bool isBaseName(const std::string& path);
    ///ditto
    bool isBaseName(const char* path);

    /// Check if path is absolute, i.e. starts with '/'.
    template<typename Iterator>
    bool isAbsolutePath(Iterator first, Iterator last) {
        return (first != last && *first == '/');
    }

    ///ditto
    bool isAbsolutePath(const std::string& path);
    ///ditto
    bool isAbsolutePath(const char* path);
}

#endif
