# Ignition Common : AV, Graphics, Events, and much more.

**Maintainer:** nate AT openrobotics DOT org

[![GitHub open issues](https://img.shields.io/github/issues-raw/ignitionrobotics/ign-common.svg)](https://github.com/ignitionrobotics/ign-common/issues)
[![GitHub open pull requests](https://img.shields.io/github/issues-pr-raw/ignitionrobotics/ign-common.svg)](https://github.com/ignitionrobotics/ign-common/pulls)
[![Discourse topics](https://img.shields.io/discourse/https/community.gazebosim.org/topics.svg)](https://community.gazebosim.org)
[![Hex.pm](https://img.shields.io/hexpm/l/plug.svg)](https://www.apache.org/licenses/LICENSE-2.0)

Build | Status
-- | --
Test coverage | [![codecov](https://codecov.io/gh/ignitionrobotics/ign-common/branch/main/graph/badge.svg)](https://codecov.io/gh/ignitionrobotics/ign-common)
Ubuntu Bionic | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=ignition_common-ci-main-bionic-amd64)](https://build.osrfoundation.org/job/ignition_common-ci-main-bionic-amd64)
Homebrew      | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=ignition_common-ci-main-homebrew-amd64)](https://build.osrfoundation.org/job/ignition_common-ci-main-homebrew-amd64)
Windows       | [![Build Status](https://build.osrfoundation.org/buildStatus/icon?job=ignition_common-ci-main-windows7-amd64)](https://build.osrfoundation.org/job/ignition_common-ci-main-windows7-amd64)

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

[Usage](#usage)

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
* **Profiler**: A common profiler abstraction that can be used to measure and
  visualize run time of various pieces of ignition robotics software.

# Install

See the [installation tutorial](https://ignitionrobotics.org/api/common/3.9/tutorials.html).

# Usage

Please refer to the [examples directory](https://github.com/ignitionrobotics/ign-common/raw/main/examples/).

# Folder Structure

Refer to the following table for information about important directories and files in this repository.

```
+-- av                       Header and source files for the AV component.
+-- events                   Header and source files for the Event component.
+-- examples                 Example programs.
+-- graphics                 Header and source files for the Graphics component.
+-- include/ignition/common  Header files for the core component.
+-- profiler                 Header and source files for the Profiler component.
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
[CONTRIBUTING.md](https://github.com/ignitionrobotics/ign-gazebo/blob/main/CONTRIBUTING.md).

# Code of Conduct

Please see
[CODE_OF_CONDUCT.md](https://github.com/ignitionrobotics/ign-gazebo/blob/main/CODE_OF_CONDUCT.md).

# Versioning

This library uses [Semantic Versioning](https://semver.org/). Additionally, this library is part of the [Ignition Robotics project](https://ignitionrobotics.org) which periodically releases a versioned set of compatible and complimentary libraries. See the [Ignition Robotics website](https://ignitionrobotics.org) for version and release information.

# License

This library is licensed under [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0). See also the [LICENSE](https://github.com/ignitionrobotics/ign-common/blob/main/LICENSE) file.

