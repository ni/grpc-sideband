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

```
> git clone https://github.com/ni/grpc-sideband.git grpc-sideband
> cd grpc-sideband
```

if you are building with RDMA support (default) then you need to install boost and rdma core
On Ubuntu you can install the following packages

```
sudo apt install libboost-all-dev
sudo apt install rdma-core librdmacm-dev
```

Build Debug - Do not build debug for profiling
```
> mkdir build
> cd build
> cmake ..
> cmake --build .
```

Build Release
```
> mkdir build
> cd build
> cmake ..
> cmake --build . --config Release
```

## Building on Linux

Download the repo, this will pull the gRPC components and all dependencies

```
> git clone https://github.com/ni/grpc-sideband.git grpc-sideband
> cd grpc-sideband
```

Build Debug - Do not build debug for profiling

```
> mkdir -p cmake/build
> cd cmake/build
> cmake ../..
> make
```

Build Release

```
> mkdir -p cmake/build
> cd cmake/build
> cmake ../..
> cmake -DCMAKE_BUILD_TYPE=Release ../..
> make
```

### RDMA Support
RDMA support can be optionally included.  It is on by default.
IF you want to disable it you can disable it from the commandline
```
> cmake -DINCLUDE_SIDEBAND_RDMA=OFF ../..
```

## Building on NI Linux RT

Install required packages not installed by default

```
> opkg update
> opkg install git
> opkg install git-perltools
> opkg install cmake
> opkg install g++
> opkg install g++-symlinks
```

Download the repo and update submodules, this will pull the gRPC components and all dependencies

```
> git clone https://github.com/ni/grpc-sideband.git grpc-sideband
> cd grpc-sideband
```

Build Debug - Do not build debug for profiling

```
> mkdir -p cmake/build
> cd cmake/build
> cmake ../..
> make
```

Build Release

```
> mkdir -p cmake/build
> cd cmake/build
> cmake -DCMAKE_BUILD_TYPE=Release ../..
> make
```
