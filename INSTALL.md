This project uses [cmake](https://cmake.org) to build.

For running the tests, you need to have [perl](https://www.perl.org).

The basic usage is
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
  
If you want to compile with custom `CFLAGS`, set them in the environment
before running `cmake`:
```sh
CFLAGS=-DMY_CUSTOM_FLAG cmake ..
```

You can get verbose build output with by passing `VERBOSE=1` to
`make`.

You can also check the [cmake FAQ](https://cmake.org/Wiki/CMake_FAQ).
