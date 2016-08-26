// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "system.h"
#include "path.h"
#include "splitter.h"

namespace mimeapps
{
    std::string findExecutable(const std::string& fileName)
    {
        if (!isBaseName(fileName)) {
            if (::access(fileName.c_str(), X_OK) == 0) {
                return fileName;
            } else {
                return std::string();
            }
        }
        
        const char* envPath = std::getenv("PATH");
        if (!envPath) {
            return std::string();
        }
        
        typedef Splitter<const char*> SplitterType;
        SplitterType splitter(envPath, envPath + std::strlen(envPath), ':');
        for (SplitterType::iterator it = splitter.begin(); it != splitter.end(); ++it) {
            std::string basePath(it->first, it->second);
            if (basePath.size()) {
                std::string filePath = buildPath(basePath, fileName);
                if (::access(filePath.c_str(), X_OK) == 0) {
                    return filePath;
                }
            }
        }
        return std::string();
    }
    
    static std::string getDETerminal() {
        const char* desktop = std::getenv("XDG_CURRENT_DESKTOP");
        if (!desktop) {
            return std::string();
        }
        if (std::strcmp(desktop, "GNOME")==0 || std::strcmp(desktop, "X-Cinnamon")==0) {
            return "gnome-terminal";
        } else if (std::strcmp(desktop, "LXDE")==0) {
            return "lxterminal";
        } else if (std::strcmp(desktop, "XFCE")==0) {
            return "xfce4-terminal";
        } else if (std::strcmp(desktop, "MATE")==0) {
            return "mate-terminal";
        } else if (std::strcmp(desktop, "KDE")==0) {
            return "konsole";
        }
        return std::string();
    }
    
    std::string getTerminal()
    {
        std::string term = findExecutable("x-terminal-emulator");
        if (!term.empty()) {
            return term;
        }
        term = findExecutable(getDETerminal());
        if (!term.empty()) {
            return term;
        }
        return findExecutable("xterm");
    }
    
    static void execProcess(char* const args[])
    {
        //close all file descriptors
        //Possible optimization: use poll and close only valid descriptors.
        struct rlimit r;
        if (getrlimit(RLIMIT_NOFILE, &r) != 0) {
            int maxDescriptors = (int)r.rlim_cur;
            int i;
            for (i=0; i<maxDescriptors; ++i) {
                ::close(i);
            }
        }
        
        //set standard streams to /dev/null
        int devNull = ::open("/dev/null", O_RDWR);
        if (devNull > 0) {
            dup2(devNull, STDIN_FILENO);
            dup2(devNull, STDOUT_FILENO);
            dup2(devNull, STDERR_FILENO);
        }
        ::close(devNull);
        
        //detach from process group of parent
        ::setsid();
        
        ::execv(args[0], args);
        //perror("execv error");
        ::_exit(1);
    }
    
    int spawnDetached(char** args)
    {
        pid_t pid = ::fork();
        if (pid == 0) {
            pid_t doubleFork = ::fork();
            if (doubleFork == 0) {
                execProcess(args);
            } else if (doubleFork < 0) {
                ::_exit(1);
            } else {
                ::_exit(0);
            }
        } else if (pid < 0) {
            return 1;
        }
        int status;
        ::waitpid(pid, &status, 0);
        return status;
    }
}
