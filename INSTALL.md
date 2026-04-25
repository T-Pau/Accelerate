To build Accelerate, you need a C++20 compiler and [cmake](https://cmake.org). To run the tests, you need [Python](https://www.python.org) and [nihtest](https://github.com/nih-at/nihtest).

The basic way to build Accelerate is
```sh
mkdir build
cd build
cmake ..
make
make test
make install
```

Some useful parameters you can pass to `cmake` with `-Dparameter=value`:

- `CMAKE_INSTALL_PREFIX`: for setting the installation path
  
If you want to compile with custom `CXXFLAGS`, set them in the environment
before running `cmake`:
```sh
CXXFLAGS=-DMY_CUSTOM_FLAG cmake ..
```

You can get verbose build output with by passing `VERBOSE=1` to
`make`.

You can also check the [cmake FAQ](https://cmake.org/Wiki/CMake_FAQ).
