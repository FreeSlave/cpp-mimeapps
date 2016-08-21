// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef MIMEAPPS_BASEDIR_H
#define MIMEAPPS_BASEDIR_H

#include <string>
#include <cstdlib>
#include <cstring>

#include "path.h"
#include "splitter.h"

namespace mimeapps
{    
    std::string configHome();
    std::string dataHome();
    
    namespace details {
        template<typename OutputIterator>
        void baseDirs(const char* envVar, const char* fallback, OutputIterator out, const char* subPath)
        {
            const char* xdgDirs = std::getenv(envVar);
            if (!xdgDirs) {
                xdgDirs = fallback;
            }
            Splitter<const char*> splitter(xdgDirs, xdgDirs + std::strlen(xdgDirs), ':');
            for (Splitter<const char*>::iterator it = splitter.begin(); it != splitter.end(); ++it) {
                if (it->first != it->second) {
                    *out = buildPath(std::string(it->first, it->second), subPath);
                }
            }
        }
    }
    
    

    template<typename OutputIterator>
    void configDirs(OutputIterator out, const char* subPath = "") {
        details::baseDirs("XDG_CONFIG_DIRS", "/etc/xdg", out, subPath);
    }

    template<typename OutputIterator>
    void dataDirs(OutputIterator out, const char* subPath = "") {
        details::baseDirs("XDG_DATA_DIRS", "/usr/local/share:/usr/share", out, subPath);
    }
}

#endif
