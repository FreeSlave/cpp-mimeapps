# Mimeapps

[![Build Status](https://travis-ci.org/MyLittleRobo/cpp-mimeapps.svg?branch=master)](https://travis-ci.org/MyLittleRobo/cpp-mimeapps)

Find and run associated applications for specific MIME-type.

This is Freedesktop only (GNU/Linux, *BSD and others).

## Building the library

```
mkdir -p build && cd build
cmake ..
make
```

Or with clang++:

```
mkdir -p build && cd build
cmake .. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS=-stdlib=libc++
make
```

## Running tests

```
mkdir -p build && cd build && cmake ..
make unittests && ctest -V
```

## Building and running examples

### Openwith-cli

Simple command line utility that lists applications associated with MIME-type and let user to choose one of them to open file.
It does not detect MIME-type automatically. By default file is considered to be text/plain type.

```
mkdir -p build && cd build && cmake ..
make openwith-cli && ./examples/openwith-cli/openwith-cli ../CMakeLists.txt 
```

Run with MIME-type hint (in this example it opens directory):

```
make openwith-cli && ./examples/openwith-cli/openwith-cli .. inode/directory
```

