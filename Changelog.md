## Ignition Common 3.x

## Ignition Common 3.X.X

1. Fix hierarchical submesh processing in the ColladaLoader.
    * [Pull Request 77](https://github.com/ignitionrobotics/ign-common/pull/77)

## Ignition Common 3.5.0 (2020-04-09)

1. Add interpolate\_x property to actor animations
    * [BitBucket pull request 232](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/232)

1. Update PBR metalness default value to 0.0
    * [BitBucket pull request 231](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/231)

1. Fix crash when loading OBJ meshes with invalid materials
    * [BitBucket pull request 230](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/230)

## Ignition Common 3.4.0 (2020-02-20)

1. Add PBR material class
    * [BitBucket pull request 227](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/227)

1. Custom paths for file URIs (env var + URI)
    * [BitBucket pull request 210](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/210)

1. Add function to get NodeAnimation
    * [BitBucket pull request 223](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/223)

1. Handle unexpected COLLADA more gracefully
    * [BitBucket pull request 218](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/218)

1. Add support for alpha based texture transparency.
    * [BitBucket pull request 221](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/221)

1. `tiny_obj_loader`: set better diffuse value when `map_kD` is present and `Kd` is not specified.
    * [BitBucket pull request 216](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/216)
    * [BitBucket pull request 217](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/217)

1. ColladaLoader: load files with actors with multiple skeletons
    * [BitBucket pull request 215](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/215)

1. Compute volume of mesh shapes with `Mesh::Volume` and `SubMesh::Volume`.
    * [BitBucket pull request 214](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/214)

1. Port Skeleton fixes from gazebo.
    * [BitBucket pull request 209](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/209)

1. Port gazebo9 fix for ColladaLoader with multiple geometries
    * [BitBucket pull request 208](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/208)

1. `SystemPaths::FindSharedLibrary`: return input if its is an absolute path to a file
    * [BitBucket pull request 206](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/206)

1. Fix `Image::ConvertPixelFormat` for `BAYER_BGGR8`
    * [BitBucket pull request 211](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/211)

1. Support custom callbacks to find files on global interface
    * [BitBucket pull request 226](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/226)

## Ignition Common 3.3.0 (2019-08-27)

1. skip GTSMeshUtils test on Windows due to issue #50
    * [BitBucket pull request 205](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/205)

1. Add bvh animation support for skeleton and TrajectoryInfo
    * [BitBucket pull request 197](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/197)

1. ColladaLoader: fix wrong node weights caused by buffer overflow bug
    * [BitBucket pull request 201](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/201)

1. Filesystem copy directory and parentpath
    * [BitBucket pull request 200](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/200)

## Ignition Common 3.2.0 (2019-08-07)

1. Allow ignLogInit to use an absolute path. Added a timeToIso that converts a given time to an ISO string. Some console timestamps were using `IGN_SYSTEM_TIME_NS()` and others `ignition::common::systemTimeISO()`. Switched all to use `ignition::common::systemTimeISO()`.
    * [BitBucket pull request 203](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/203)

1. Port ColladaLoader fix: use default value of 1 for stride parameter when unset.
    * [BitBucket pull request 196](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/196)

## Ignition Common 3.1.0 (2019-05-17)

1. Image::PixelFormatType: append `BAYER_BGGR8` instead of replacing `BAYER_RGGR8`
    * [BitBucket pull request 192](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/192)

1. Use `std::tolower` with locale in lowercase helper function
    * [BitBucket pull request 190](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/190)

1. Profiler: give hint on osx to find frameworks so it doesn't find them inside Xcode
    * [BitBucket pull request 189](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/189)

1. Fix windows tests: SystemPathsFixture.FileSystemPaths and TimeTest.Sleep
    * [BitBucket pull request 188](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/188)

1. Filesystem functions to create unique paths
    * [BitBucket pull request 187](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/187)

1. Battery additions for LinearBatteryPlugin port to ign-gazebo
    * [BitBucket pull request 186](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/186)

1. Consolidate Console::log streams to reduce colored log size
    * [BitBucket pull request 185](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/185)
    * [Issue 47](https://github.com/ignitionrobotics/ign-common/issues/47)

1. Fix ColladaLoader to support mixamo models and fix skeleton animation loading
    * [BitBucket pull request 179](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/179)
    * [BitBucket osrf/gazebo pull request 3071](https://osrf-migration.github.io/gazebo-gh-pages/#!/osrf/gazebo/pull-requests/3071)

## Ignition Common 3.0.0 (2019-02-28)

1. Use ign-cmake2 and support Ubuntu Bionic (18.04).
    * [BitBucket pull request 131](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/131)

1. WorkerPool: allow user to set minimum number of worker threads.
    * [BitBucket pull request 151](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/151)

1. ColladaLoader fixes: setting multiple texcoord's, check for line breaks when using `split`
    * [BitBucket pull request 152](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/152)
    * [BitBucket pull request 157](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/157)

1. Event: replace `operator()` and `Signal()` explicit templates with variadic templates and perfect forwarding.
    * [BitBucket pull request 156](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/156)

1. Event: use second template argument to disambiguate events with matching signatures.
    * [BitBucket pull request 164](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/164)
    * [BitBucket pull request 166](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/166)

1. Add profiler using Remotery.
    * [BitBucket pull request 162](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/162)
    * [BitBucket pull request 169](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/169)

1. Added hash64 function for converting `std::string` to `uint64_t` as `constexpr`.
    * [BitBucket pull request 170](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/170)
    * [BitBucket pull request 171](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/171)
    * [BitBucket pull request 172](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/172)

1. SystemPaths: search paths in `IGN_FILE_PATH` environment variable when finding files
    * [BitBucket pull request 175](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/175)

1. `Time::Sleep`: return amount of time actually slept
    * [BitBucket pull request 175](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/176)
    * [Issue 44](https://github.com/ignitionrobotics/ign-common/issues/44)

1. NodeTransform: use unique_ptr for private data, add copy constructor/assignment
    * [BitBucket pull request 181](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/181)

## Ignition Common 2.x

## Ignition Common 2.x.x (2018-XX-XX)

## Ignition Common 2.0.0 (2018-02-11)

1. Use ignition-cmake1 and components for av, events, and graphics
    * [BitBucket pull request 102](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/102)
    * [BitBucket pull request 103](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/103)
    * [BitBucket pull request 105](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/105)
    * [BitBucket pull request 106](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/106)
    * [BitBucket pull request 107](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/107)
    * [BitBucket pull request 108](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/108)
    * [BitBucket pull request 109](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/109)

1. Expand plugin functionality with PluginPtr and specializations
    * [BitBucket pull request 59](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/59)

1. Added signal handler class
    * [BitBucket pull request 115](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/115)

1. Update tinyobjloader to version that supports triangulating meshes with concave polygons.
    * [BitBucket pull request 160](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/160)

## Ignition Common 1.x.x (2018-XX-XX)


## Ignition Common 1.X.X (20XX-XX-XX)

1. Time::Sleep: use CLOCK_MONOTONIC on Linux
    * [BitBucket pull request 159](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/159)


## Ignition Common 1.1.1 (2018-05-23)

1. Documentation upload improvements
    * [BitBucket pull request 117](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/117)

1. Fix build on homebrew with ffmpeg4
    * [BitBucket pull request 119](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/119)

1. Remove symbolic link directory
    * [BitBucket pull request 124](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/124)


## Ignition Common 1.1.0 (2018-04-16)

1. Accept spaces on URI path
    * [BitBucket pull request 110](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/110)
    * [BitBucket pull request 111](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/111)

1. Fix gcc7 compiler warnings on artful
    * [BitBucket pull request 97](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/97)

1. Fix artful cppcheck
    * [BitBucket pull request 99](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/99)

1. Fix nanosecond to second conversion
    * [BitBucket pull request 100](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/100)
