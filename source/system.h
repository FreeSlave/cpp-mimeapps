// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef MIMEAPPS_SYSTEM_H
#define MIMEAPPS_SYSTEM_H

#include <string>
#include <vector>

#include <errno.h>

#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <cstdio>

namespace mimeapps
{
    std::string findExecutable(const std::string& baseName);
    std::string getTerminal(std::string& arg);

    struct SystemError
    {
        SystemError(int code, const char* msg) : errorMsg(msg), status(code) {}

        const char* errorMsg;
        int status;
    };

    SystemError spawnDetached(char** args, const char* workingDirectory = NULL, unsigned int* pid = NULL);

    template<typename Iterator>
    SystemError spawnDetached(const Iterator& first, const Iterator& last)
    {
        if (first == last) {
            return SystemError(EINVAL, "Empty argument list");
        }

        std::vector<std::vector<char> > argv;
        for (Iterator it = first; it != last; ++it) {
            std::vector<char> arg(it->size()+1);
            std::strcpy(&arg[0], it->c_str());
            argv.push_back(arg);
        }

        std::vector<char*> args(argv.size()+1);
        for (std::size_t i = 0; i<argv.size(); ++i) {
            args[i] = &argv[i][0];
        }
        args[argv.size()] = 0;

        return spawnDetached(&args[0]);
    }
}

#endif
