\page install Installation

Next Tutorial: \ref profiler

# Install

These instructions are for installing only Ignition Common.
If you're interested in using all the Ignition libraries, check out this [Ignition installation](https://ignitionrobotics.org/docs/latest/install).

We recommend following the Binary Install instructions to get up and running as quickly and painlessly as possible.

The Source Install instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

## Binary Install

### Ubuntu

On Ubuntu systems, `apt-get` can be used to install `ignition-common`:

```
sudo apt install libignition-common<#>-dev
```

Be sure to replace `<#>` with a number value, such as 2 or 3, depending on
which version you need.

### Windows

Install [Conda package management system](https://docs.conda.io/projects/conda/en/latest/user-guide/install/download.html).
Miniconda suffices.

Create if necessary, and activate a Conda environment:

```
conda create -n ign-ws
conda activate ign-ws
```

Install `ignition-common`:

```
conda install libignition-common<#> --channel conda-forge
```

Be sure to replace `<#>` with a number value, such as 2 or 3, depending on
which version you need.

## Source Install

Source installation can be performed by first installing the necessary
prerequisites followed by building from source.

### Prerequisites

Ignition Common requires:

  * [Ignition CMake](https://ignitionrobotics.org/libs/cmake)
  * [Ignition Math](https://ignitionrobotics.org/libs/math).

The Graphics component requires:

  * [FreeImage](http://freeimage.sourceforge.net/)
  * [GTS](http://gts.sourceforge.net/).

The AV component requires:

  * [libswscale](https://www.ffmpeg.org/libswscale.html)
  * [libavdevice](https://www.ffmpeg.org/libavdevice.html)
  * [libavformat](https://www.ffmpeg.org/libavformat.html)
  * [libavcodec](https://www.ffmpeg.org/libavcodec.html)
  * [libavutil](https://www.ffmpeg.org/libavutil.html)

#### Windows

First, follow the [ign-cmake](https://github.com/ignitionrobotics/ign-cmake) tutorial for installing Conda, Visual Studio, CMake, etc., prerequisites, and creating a Conda environment.

Navigate to `condabin` if necessary to use the `conda` command (i.e., if Conda is not in your `PATH` environment variable. You can find the location of `condabin` in Anaconda Prompt, `where conda`).

Activate the Conda environment:

```
conda activate ign-ws
```

Install prerequisites:

```
conda install freeimage gts glib dlfcn-win32 ffmpeg --channel conda-forge
```

Install Ignition dependencies:

You can view available versions and their dependencies:

```
conda search libignition-common* --channel conda-forge --info
```

Install dependencies, replacing `<#>` with the desired versions:

```
conda install libignition-cmake<#> libignition-math<#> --channel conda-forge
```

### Building from source

#### Ubuntu

1. Clone the repository
  ```
  git clone https://github.com/ignitionrobotics/ign-common
  ```

1. Configure and build
  ```
  cd ign-common
  mkdir build
  cd build
  cmake ..
  make
  ```

1. Optionally, install Ignition Common
  ```
  sudo make install
  ```

#### Windows

1. Navigate to where you would like to build the library, and clone the repository.
  ```
  # Optionally, append `-b ign-common#` (replace # with a number) to check out a specific version
  git clone https://github.com/ignitionrobotics/ign-common.git
  ```

1. Configure and build
  ```
  cd ign-common
  mkdir build
  cd build
  cmake .. -DBUILD_TESTING=OFF  # Optionally, -DCMAKE_INSTALL_PREFIX=path\to\install
  cmake --build . --config Release
  ```

1. Optionally, install
  ```
  cmake --install . --config Release
  ```
