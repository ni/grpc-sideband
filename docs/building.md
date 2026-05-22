## Building on Windows

### Prerequisites
To prepare for cmake + Microsoft Visual C++ compiler build
- Install Visual Studio 2015, 2017, or 2019 (Visual C++ compiler will be used).
- Install [Git](https://git-scm.com/).
- Install gRPC for C++
- Install [CMake](https://cmake.org/download/).


### Building
- Launch "x64 Native Tools Command Prompt for Visual Studio"

Download the repo and update submodules, this will pull the gRPC components and all dependencies

```bash
git clone https://github.com/ni/grpc-sideband.git grpc-sideband
cd grpc-sideband
```

If you are building with RDMA support (default) then you need to install boost and rdma core.
On Ubuntu you can install the following packages:

```bash
sudo apt install libboost-all-dev
sudo apt install rdma-core librdmacm-dev
```

Build Debug - Do not build debug for profiling
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Build Release
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Building on Linux

Download the repo, this will pull the gRPC components and all dependencies

```bash
git clone https://github.com/ni/grpc-sideband.git grpc-sideband
cd grpc-sideband
```

Build Debug - Do not build debug for profiling

```bash
mkdir -p cmake/build
cd cmake/build
cmake ../..
make
```

Build Release

```bash
mkdir -p cmake/build
cd cmake/build
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
```

### Submodule Dependencies

By default, the build uses bundled third-party libraries from the `third_party/` directory. To use
distribution-provided libraries instead (e.g. when building in OpenEmbedded or with a system package
manager):

```bash
cmake -DUSE_SUBMODULE_DEPENDENCIES=OFF ../..
```

### RDMA Support

RDMA support is included by default. To disable it:

```bash
cmake -DINCLUDE_SIDEBAND_RDMA=OFF ../..
```

### Installing

To install to the default system prefix (`/usr/local`):

```bash
cmake --install cmake/build
```

To install to a custom prefix:

```bash
cmake --install cmake/build --prefix /usr
```

To install into a staging directory (e.g. for packaging) without touching the host system, use the
`DESTDIR` environment variable:

```bash
DESTDIR=/path/to/staging cmake --install cmake/build --prefix /usr
```

## Building on NI Linux RT

Install required packages not installed by default

```bash
opkg update
opkg install git
opkg install git-perltools
opkg install cmake
opkg install g++
opkg install g++-symlinks
```

Download the repo and update submodules, this will pull the gRPC components and all dependencies

```bash
git clone https://github.com/ni/grpc-sideband.git grpc-sideband
cd grpc-sideband
```

Build Debug - Do not build debug for profiling

```bash
mkdir -p cmake/build
cd cmake/build
cmake ../..
make
```

Build Release

```bash
mkdir -p cmake/build
cd cmake/build
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
```

### Installing

To install to a standard prefix on the target device:

```bash
cmake --install cmake/build --prefix /usr/local
```

