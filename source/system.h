// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef MIMEAPPS_SYSTEM_H
#define MIMEAPPS_SYSTEM_H

#include <string>
#include <vector>

#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <cstdio>

namespace mimeapps
{
    std::string findExecutable(const std::string& baseName);
    std::string getTerminal();
    
    int spawnDetached(char** args);
    
    template<typename Iterator>
    int spawnDetached(const Iterator& first, const Iterator& last)
    {
        if (first == last) {
            return 1;
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
