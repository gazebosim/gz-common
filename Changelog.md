## Ignition Common 3.x

## Ignition Common 3.X.X

1. Add PBR material class
    * [Pull request 227](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/227)

1. Custom paths for file URIs (env var + URI)
    * [Pull request 210](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/210)

1. Add function to get NodeAnimation
    * [Pull request 223](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/223)

1. Add support for alpha based texture transparency.
    * [Pull request 221](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/221)

1. `tiny_obj_loader`: set better diffuse value when `map_kD` is present and `Kd` is not specified.
    * [Pull request 216](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/216)
    * [Pull request 217](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/217)

1. ColladaLoader: load files with actors with multiple skeletons
    * [Pull request 215](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/215)

1. Compute volume of mesh shapes with `Mesh::Volume` and `SubMesh::Volume`.
    * [Pull request 214](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/214)

1. Port Skeleton fixes from gazebo.
    * [Pull request 209](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/209)

1. Port gazebo9 fix for ColladaLoader with multiple geometries
    * [Pull request 208](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/208)

1. `SystemPaths::FindSharedLibrary`: return input if its is an absolute path to a file
    * [Pull request 206](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/206)

1. Fix `Image::ConvertPixelFormat` for `BAYER_BGGR8`
    * [Pull request 211](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/211)

1. Support custom callbacks to find files on global interface
    * [Pull request 226](https://bitbucket.org/ignitionrobotics/ign-common/pull-request/226)

## Ignition Common 3.3.0 (2019-08-27)

1. skip GTSMeshUtils test on Windows due to issue #50
    * [Pull request 205](https://bitbucket.org/osrf/gazebo/pull-request/205)

1. Add bvh animation support for skeleton and TrajectoryInfo
    * [Pull request 197](https://bitbucket.org/osrf/gazebo/pull-request/197)

1. ColladaLoader: fix wrong node weights caused by buffer overflow bug
    * [Pull request 201](https://bitbucket.org/osrf/gazebo/pull-request/201)

1. Filesystem copy directory and parentpath
    * [Pull request 200](https://bitbucket.org/osrf/gazebo/pull-request/200)

## Ignition Common 3.2.0 (2019-08-07)

1. Allow ignLogInit to use an absolute path. Added a timeToIso that converts a given time to an ISO string. Some console timestamps were using `IGN_SYSTEM_TIME_NS()` and others `ignition::common::systemTimeISO()`. Switched all to use `ignition::common::systemTimeISO()`.
    * [Pull request 203](https://bitbucket.org/osrf/gazebo/pull-request/203)

1. Port ColladaLoader fix: use default value of 1 for stride parameter when unset.
    * [Pull request 196](https://bitbucket.org/osrf/gazebo/pull-request/196)

## Ignition Common 3.1.0 (2019-05-17)

1. Image::PixelFormatType: append `BAYER_BGGR8` instead of replacing `BAYER_RGGR8`
    * [Pull request 192](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/192)

1. Use `std::tolower` with locale in lowercase helper function
    * [Pull request 190](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/190)

1. Profiler: give hint on osx to find frameworks so it doesn't find them inside Xcode
    * [Pull request 189](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/189)

1. Fix windows tests: SystemPathsFixture.FileSystemPaths and TimeTest.Sleep
    * [Pull request 188](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/188)

1. Filesystem functions to create unique paths
    * [Pull request 187](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/187)

1. Battery additions for LinearBatteryPlugin port to ign-gazebo
    * [Pull request 186](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/186)

1. Consolidate Console::log streams to reduce colored log size
    * [Pull request 185](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/185)
    * [Issue 47](https://bitbucket.org/ignitionrobotics/ign-common/issue/47)

1. Fix ColladaLoader to support mixamo models and fix skeleton animation loading
    * [Pull request 179](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/179)
    * [osrf/gazebo pull request 3071](https://bitbucket.org/osrf/gazebo/pull-requests/3071)

## Ignition Common 3.0.0 (2019-02-28)

1. Use ign-cmake2 and support Ubuntu Bionic (18.04).
    * [Pull request 131](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/131)

1. WorkerPool: allow user to set minimum number of worker threads.
    * [Pull request 151](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/151)

1. ColladaLoader fixes: setting multiple texcoord's, check for line breaks when using `split`
    * [Pull request 152](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/152)
    * [Pull request 157](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/157)

1. Event: replace `operator()` and `Signal()` explicit templates with variadic templates and perfect forwarding.
    * [Pull request 156](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/156)

1. Event: use second template argument to disambiguate events with matching signatures.
    * [Pull request 164](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/164)
    * [Pull request 166](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/166)

1. Add profiler using Remotery.
    * [Pull request 162](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/162)
    * [Pull request 169](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/169)

1. Added hash64 function for converting `std::string` to `uint64_t` as `constexpr`.
    * [Pull request 170](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/170)
    * [Pull request 171](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/171)
    * [Pull request 172](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/172)

1. SystemPaths: search paths in `IGN_FILE_PATH` environment variable when finding files
    * [Pull request 175](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/175)

1. `Time::Sleep`: return amount of time actually slept
    * [Pull request 175](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/175)
    * [Issue 44](https://bitbucket.org/ignitionrobotics/ign-common/issue/44)

1. NodeTransform: use unique_ptr for private data, add copy constructor/assignment
    * [Pull request 181](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/181)

## Ignition Common 2.x

## Ignition Common 2.x.x (2018-XX-XX)

## Ignition Common 2.0.0 (2018-02-11)

1. Use ignition-cmake1 and components for av, events, and graphics
    * [Pull request 102](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/102)
    * [Pull request 103](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/103)
    * [Pull request 105](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/105)
    * [Pull request 106](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/106)
    * [Pull request 107](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/107)
    * [Pull request 108](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/108)
    * [Pull request 109](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/109)

1. Expand plugin functionality with PluginPtr and specializations
    * [Pull request 59](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/59)

1. Added signal handler class
    * [Pull request 115](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/115)

1. Update tinyobjloader to version that supports triangulating meshes with concave polygons.
    * [Pull request 160](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/160)

## Ignition Common 1.x.x (2018-XX-XX)


## Ignition Common 1.X.X (20XX-XX-XX)

1. Time::Sleep: use CLOCK_MONOTONIC on Linux
    * [Pull request 159](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/159)


## Ignition Common 1.1.1 (2018-05-23)

1. Documentation upload improvements
    * [Pull request 117](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/117)

1. Fix build on homebrew with ffmpeg4
    * [Pull request 119](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/119)

1. Remove symbolic link directory
    * [Pull request 124](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/124)


## Ignition Common 1.1.0 (2018-04-16)

1. Accept spaces on URI path
    * [Pull request 110](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/110)
    * [Pull request 111](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/111)

1. Fix gcc7 compiler warnings on artful
    * [Pull request 97](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/97)

1. Fix artful cppcheck
    * [Pull request 99](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/99)

1. Fix nanosecond to second conversion
    * [Pull request 100](https://bitbucket.org/ignitionrobotics/ign-common/pull-requests/100)
