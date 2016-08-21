#!/bin/sh
cppcheck --std=c++03 --std=posix --enable=warning --enable=portability --language=c++ -I source --force --quiet source
