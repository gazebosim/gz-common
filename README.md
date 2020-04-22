# Ignition Common : AV, Graphics, Events, and much more.

**Maintainer:** nate AT openrobotics DOT org

[![GitHub open issues](https://img.shields.io/github/issues-raw/ignitionrobotics/ign-common.svg)](https://github.com/ignitionrobotics/ign-common/issues)
[![GitHub open pull requests](https://img.shields.io/github/issues-pr-raw/ignitionrobotics/ign-common.svg)](https://github.com/ignitionrobotics/ign-common/pulls)
[![Discourse topics](https://img.shields.io/discourse/https/community.gazebosim.org/topics.svg)](https://community.gazebosim.org)
[![Hex.pm](https://img.shields.io/hexpm/l/plug.svg)](https://www.apache.org/licenses/LICENSE-2.0)

Build | Status
-- | --
Test coverage | [![codecov](https://codecov.io/bb/ignitionrobotics/ign-common/branch/default/graph/badge.svg)](https://codecov.io/bb/ignitionrobotics/ign-common)
Ubuntu Bionic | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=ignition_common-ci-default-bionic-amd64)](https://build.osrfoundation.org/job/ignition_common-ci-default-bionic-amd64)
Homebrew      | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=ignition_common-ci-default-homebrew-amd64)](https://build.osrfoundation.org/job/ignition_common-ci-default-homebrew-amd64)
Windows       | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=ignition_common-ci-default-windows7-amd64)](https://build.osrfoundation.org/job/ignition_common-ci-default-windows7-amd64)

Ignition Common, a component of [Ignition
Robotics](https://ignitionrobotics.org), provides a set of libraries that
cover many different use cases. An audio-visual library supports
processing audio and video files, a graphics library can load a variety 3D
mesh file formats into a generic in-memory representation, and the core
library of Ignition Common contains functionality that spans Base64
encoding/decoding to thread pools.

# Table of Contents

[Features](#features)

[Install](#install)

* [Binary Install](#binary-install)

* [Source Install](#source-install)

    * [Prerequisites](#prerequisites)

    * [Building from Source](#building-from-source)

[Usage](#usage)

[Documentation](#documentation)

[Testing](#testing)

[Folder Structure](#folder-structure)

[Code of Conduct](#code-of-conduct)

[Contributing](#code-of-contributing)

[Versioning](#versioning)

[License](#license)

# Features

Some of the many capabilities contained in Ignition Common are:

* **AV**: FFMpeg based audio decoder, and video encoder and decoder.
* **Core**: Base64 encoding and decoding, battery model, console logging,
  cross-platform filesystem interface, URI processing, and a thread pool.
* **Events**: Mouse and keyboard events, and a high-performance signal and
callback system.
* **Graphics**: Collada, SVG, STL, OBJ, and DEM loaders. In-memory mesh,
  image, and material representations. Animation processing and BVH loader.

# Install

We recommend following the [Binary Install](#binary-install) instructions to get up and running as quickly and painlessly as possible.

The [Source Install](#source-install) instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

## Binary Install

On Ubuntu systems, `apt-get` can be used to install `ignition-common`:

```
sudo apt install libignition-common<#>-dev
```

Be sure to replace `<#>` with a number value, such as 2 or 3, depending on
which version you need.

## Source Install

Source installation can be performed in UNIX systems by first installing the
necessary prerequisites followed by building from source.

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

### Building from source

1. Clone the repository

    ```
    git clone https://github.com/ignitionrobotics/ign-common
    ```

2. Install the [Prerequisites](#prerequisites).

3. Configure and build

    ```
    cd ign-common; mkdir build;cd build; cmake ..;  make
    ```

4. Optionally, install Ignition Common

    ```
    sudo make install
    ```

# Usage

Please refer to the [examples directory](https://github.com/ignitionrobotics/ign-common/raw/master/examples/).

# Documentation

API and tutorials can be found at [https://ignitionrobotics.org/libs/common](https://ignitionrobotics.org/libs/common).

You can also generate the documentation from a clone of this repository by following these steps.

1. You will need Doxygen. On Ubuntu Doxygen can be installed using

    ```
    sudo apt-get install doxygen
    ```

2. Clone the repository

    ```
    git clone https://github.com/ignitionrobotics/ign-common
    ```

3. Configure and build the documentation.

    ```
    cd ign-common; mkdir build; cd build; cmake ../; make doc
    ```

4. View the documentation by running the following command from the build directory.

    ```
    firefox doxygen/html/index.html
    ```

# Testing

Follow these steps to run tests and static code analysis in your clone of this repository.

1. Follow the [source install instruction](#source-install).

2. Run tests.

    ```
    make test
    ```

3. Static code checker.

    ```
    make codecheck
    ```

# Folder Structure

Refer to the following table for information about important directories and files in this repository.

```
+-- av                       Header and source files for the AV component.
+-- events                   Header and source files for the Event component.
+-- examples                 Example programs.
+-- graphics                 Header and source files for the Graphics component.
+-- include/ignition/common  Header files for the core component.
+-- src                      Core source files and unit tests.
+-- test
|    +-- integration         Integration tests.
|    +-- performance         Performance tests.
|    +-- plugins             Plugin tests.
|    +-- static_assertions   Plugin static assertion tests.
|    +-- regression          Regression tests.
+-- tutorials                Tutorials, written in markdown.
+-- Changelog.md             Changelog.
+-- CMakeLists.txt           CMake build script.
+-- Migration.md             Migration guide.
```
# Contributing

Please see
[CONTRIBUTING.md](https://github.com/ignitionrobotics/ign-gazebo/blob/master/CONTRIBUTING.md).

# Code of Conduct

Please see
[CODE_OF_CONDUCT.md](https://github.com/ignitionrobotics/ign-gazebo/blob/master/CODE_OF_CONDUCT.md).

# Versioning

This library uses [Semantic Versioning](https://semver.org/). Additionally, this library is part of the [Ignition Robotics project](https://ignitionrobotics.org) which periodically releases a versioned set of compatible and complimentary libraries. See the [Ignition Robotics website](https://ignitionrobotics.org) for version and release information.

# License

This library is licensed under [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0). See also the [LICENSE](https://github.com/ignitionrobotics/ign-common/blob/master/LICENSE) file.
