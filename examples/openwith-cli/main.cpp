// Copyright (c) 2016 Roman Chistokhodov
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <iostream>
#include <iterator>
#include <vector>
#include <cstdio>
#include <cstddef>
#include "mimeapps.h"

using namespace mimeapps;

int main(int argc, char** argv)
{
    std::string mimeTypeHint = "text/plain";
    std::string filePath;

    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <path> [mime-type]\n", argv[0]);
        return 1;
    } else {
        filePath = argv[1];
    }
    if (argc > 2) {
        mimeTypeHint = argv[2];
    }

    DesktopFile defaultApp = findDefaultApplication(mimeTypeHint);

    if (!defaultApp.isValid()) {
        std::cerr << "Could not find applications to open file of type " << mimeTypeHint << std::endl;
        return 1;
    }

    std::vector<DesktopFile> apps;
    findAssociatedApplications(mimeTypeHint, std::back_inserter(apps));

    std::cout << "Choose application to open " << filePath << ":\n";
    std::cout << "\t0: " << defaultApp.name() << " (" << defaultApp.fileName() << ") - default\n";
    for (std::size_t i=0; i<apps.size(); ++i) {
        std::cout << '\t' << (i+1) << ": " << apps[i].name() << " (" << apps[i].fileName() << ")\n";
    }

    int index = 0;
    std::cout << "Type the number of application: ";
    std::cin >> index;
    if (index > apps.size() || index < 0) {
        std::cerr << "Wrong number" << std::endl;
        return 1;
    }

    try {
        if (index == 0) {
            defaultApp.spawnApplication(filePath);
        } else {
            apps[index-1].spawnApplication(filePath);
        }
        return 0;
    } catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
