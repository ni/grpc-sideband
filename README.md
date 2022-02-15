# gRPC sideband data support

the project supports Windows, Linux and Linux RT.

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

Build - Do not build debug for profiling

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

## Building on Linux RT

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
