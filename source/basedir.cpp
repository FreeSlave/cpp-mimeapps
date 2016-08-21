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
