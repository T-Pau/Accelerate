# Building Accelerate

This guide shows how to build Accelerate source. It assumes basic familiarity with using the command line and building software from source.

This guide uses cmake to invoke all required commands to be platform-independent. You can, of course, also use the underlying commands directly.


## Dependencies

Accelerate requires a C++20 compiler. It has been tested with gcc, clang, and MSVC. It also requires [CMake](https://cmake.org/) (version 3.10 or later) and any of the supported build tools like Ninja, Make, or Visual Studio.

It also requires [T'Pau C++ Kernal](https://tpau-cpp-kernal.tpau.group/) to be installed.

For running the tests, you need to have [Python](https://www.python.org/), and [nihtest](https://pypi.org/project/nihtest/) installed.

How to install the dependencies depends on your operating system. On Linux, you can usually install them via your package manager. On macOS, you can use Homebrew or Mac Ports. On Windows, you can use vcpkg. Details on how to use these package managers is beyond the scope of this guide, but should be easy to find online.


## Building Accelerate

### 1. Clone the repository and navigate to the top level directory.

Since there are no official releases yet, you will have to clone the repository.

```sh
git clone https://github.com/T-Pau/Accelerate.git
cd Accelerate
```


### 2. Create a build directory and navigate to it.

```sh
cmake -E make_directory build
cd build
```


### 3. Run cmake to configure the build system.

```sh
cmake ..
```
If you want to customize the build, you can pass additional parameters to cmake after `..` in the form of `-Dparameter=value`. See below for some useful parameters:

`CMAKE_INSTALL_PREFIX`
: for setting the installation path
  
If you want to compile with custom `CFLAGS`, set them in the environment before running `cmake`:
```sh
CFLAGS=-DMY_CUSTOM_FLAG cmake ..
```

You can also check the [cmake FAQ](https://gitlab.kitware.com/cmake/community/-/wikis/FAQ) for more information.


### 4. Build Accelerate.

```sh
cmake --build .
```


### 5. Run the test suite (optional).

To check that Accelerate works as expected, run the test suite.

```sh
ctest -j20
```

The number after `-j` specifies how many tests to run in parallel. 20 is a good default for modern systems.


### 6. Install Accelerate.

```sh
cmake --install .
```

Installing to the default location may require root privileges. You can specify a different installation location by passing `-DCMAKE_INSTALL_PREFIX=/path/to/install` to cmake in step 3.
