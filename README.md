# Mimeapps


[![Build Status](https://github.com/FreeSlave/cpp-mimeapps/actions/workflows/.github.yml?branch=master)](https://github.com/FreeSlave/cpp-mimeapps/actions/workflows/.github.yml)

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

You need Boost Test Library to build and run tests:

```
sudo apt-get install libboost-test-dev # On Debian, Ubuntu, etc.
```

Then run:

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

### Openwith-qt

Similar program, but with Qt gui. Go to examples/openwith-qt and open openwith-qt.pro in QtCreator.
