// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include "basedir.h"

namespace mimeapps
{
    static std::string xdgHomeDir(const char* envVar, const char* fallback) {
        const char* envValue = std::getenv(envVar);
        if (envValue) {
            return envValue;
        } else {
            const char* home = std::getenv("HOME");
            if (!home) {
                home = ".";
            }
            return buildPath(home, fallback);
        }
    }

    std::string configHome() {
        return xdgHomeDir("XDG_CONFIG_HOME", ".config");
    }

    std::string dataHome() {
        return xdgHomeDir("XDG_DATA_HOME", ".local/share");
    }
}
