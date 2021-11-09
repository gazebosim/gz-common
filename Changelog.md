## Ignition Common 4.x

## Ignition Common 4.4.0 (2021-10-15)

1. Add support for animation tension 
    * [Pull request #256](https://github.com/ignitionrobotics/ign-common/pull/256)

## Ignition Common 4.3.0 (2021-09-27)

1. Remove ign-utils from TempDirectory
    * [Pull request #248](https://github.com/ignitionrobotics/ign-common/pull/248)

1. Add functions and objects for Temporary Directories
    * [Pull request #244](https://github.com/ignitionrobotics/ign-common/pull/244)

1. Fix memory corruption & leaks in Image
    * [Pull request #240](https://github.com/ignitionrobotics/ign-common/pull/240)

1. Fix a typo in VideoEncoder_TEST.
    * [Pull request #231](https://github.com/ignitionrobotics/ign-common/pull/231)

1. Fix segfault caused by destruction order of Event and Connection
    * [Pull request #234](https://github.com/ignitionrobotics/ign-common/pull/234)

1. Infrastructure
    * [Pull request #62](https://github.com/ignitionrobotics/ign-common/pull/62)
    * [Pull request #55](https://github.com/ignitionrobotics/ign-common/pull/55)
    * [Pull request #241](https://github.com/ignitionrobotics/ign-common/pull/241)
  
1. Documentation
    * [Pull request #252](https://github.com/ignitionrobotics/ign-common/pull/252)
    * [Pull request #253](https://github.com/ignitionrobotics/ign-common/pull/253)

## Ignition Common 4.2.0 (2021-08-02)

1. Export lights to dae
    * [Pull request #228](https://github.com/ignitionrobotics/ign-common/pull/228)

1. Add cstring for std::memcpy
    * [Pull request #230](https://github.com/ignitionrobotics/ign-common/pull/230)

## Ignition Common 4.1.0 (2021-06-22)

1. Make KeyEvent rule-of-five compliant
    * [Pull request #224](https://github.com/ignitionrobotics/ign-common/pull/224)

1. Fix `av_*` API usage for deprecations
    * [Pull request #220](https://github.com/ignitionrobotics/ign-common/pull/220)

1. Set project-wide standard to C++17
    * [Pull request #221](https://github.com/ignitionrobotics/ign-common/pull/221)

1. Remove `CMAKE_CXX_FLAGS` from test targets
    * [Pull request #214](https://github.com/ignitionrobotics/ign-common/pull/214)

1. Support loading PBR textures in OBJLoader
    * [Pull request #216](https://github.com/ignitionrobotics/ign-common/pull/216)

1. Remove `tools/code_check` and update codecov
    * [Pull request #219](https://github.com/ignitionrobotics/ign-common/pull/219)

1. Port codecov to new configuration
    * [Pull request #212](https://github.com/ignitionrobotics/ign-common/pull/212)

1. Fix loading collada files with multiple texcoord sets using the same offset
    * [Pull request #208](https://github.com/ignitionrobotics/ign-common/pull/208)

1. Add function to convert single channel image data to RGB image
    * [Pull request #205](https://github.com/ignitionrobotics/ign-common/pull/205)

1. Remove ColladaExporter path constraint
    * [Pull request #204](https://github.com/ignitionrobotics/ign-common/pull/204)

1. Avoid duplication of / in joinPaths (Windows)
    * [Pull request #201](https://github.com/ignitionrobotics/ign-common/pull/201)
    * [Pull request #209](https://github.com/ignitionrobotics/ign-common/pull/209)
    * [Pull request #215](https://github.com/ignitionrobotics/ign-common/pull/215)

1. Fixed colladaLoader on Windows
    * [Pull request #200](https://github.com/ignitionrobotics/ign-common/pull/200)

1. Improved Windows support
    * [Pull request #197](https://github.com/ignitionrobotics/ign-common/pull/197)

1. Function to serialize compressed image to buffer
    * [Pull request #139](https://github.com/ignitionrobotics/ign-common/pull/139)

1. Fixed tutorial link
    * [Pull request #194](https://github.com/ignitionrobotics/ign-common/pull/194)

1. Use matching malloc/free for AudioDecoder.
    * [Pull request #192](https://github.com/ignitionrobotics/ign-common/pull/192)

1. Add bazel buildsystem support
    * [Pull request #123](https://github.com/ignitionrobotics/ign-common/pull/123)

## Ignition Common 4.0.0 (2021-03-30)

1. Include windows instructions in examples README.md
    * [Pull request #190](https://github.com/ignitionrobotics/ign-common/pull/190)

1. Fix crash when using BVH animations
    * [Pull request #188](https://github.com/ignitionrobotics/ign-common/pull/188)

1. Rotate capsule to Z-up
    * [Pull request #186](https://github.com/ignitionrobotics/ign-common/pull/186)

1. Add implementation pointer to ign-common classes
    * [Pull request #163](https://github.com/ignitionrobotics/ign-common/pull/163)

1. Cleanup and add implementation pointer to Animation classes
    * [Pull request #160](https://github.com/ignitionrobotics/ign-common/pull/160)

1. Install the profiler in a version-safe manner
    * [Pull request #170](https://github.com/ignitionrobotics/ign-common/pull/170)

1. Restore support for model:// URIs
    * [Pull request #172](https://github.com/ignitionrobotics/ign-common/pull/172)

1. Deprecated common::Time
    * [Pull request #90](https://github.com/ignitionrobotics/ign-common/pull/90)

1. Update codeowners
    * [Pull request #88](https://github.com/ignitionrobotics/ign-common/pull/88)

1. Workflow updates
    * [Pull request #64](https://github.com/ignitionrobotics/ign-common/pull/64)

1. Update BitBucket Links
    * [Pull request #58](https://github.com/ignitionrobotics/ign-common/pull/58)

1. Corrected `BAYER_RGGR8` to `BAYER_BGGR8` in `PixelFormatName` and
   `PixelFormatType` located in `graphics/include/ignition/common/Image.hh`.
    * [BitBucket pull request 191](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/191)

1. Added URI Authority parsing to the URI class. Authority information can
   be found [here](https://en.wikipedia.org/wiki/Uniform_Resource_Identifier#Generic_syntax). This update also caused some formally valid syntax to become invalid, and some formally invalid syntax to become valid. See the migration guide.
    * [BitBucket pull request xxx](https://osrf-migration.github.io/ignition-gh-pages/#!/ignitionrobotics/ign-common/pull-requests/213)

## Ignition Common 3.x

## Ignition Common 3.X.X

## Ignition Common 3.13.2 (2021-05-11)

1. Backport collada fixes (Backport #204)
    * [Pull request #211](https://github.com/ignitionrobotics/ign-common/pull/211)

1. Fix join paths again
    * [Pull request #215](https://github.com/ignitionrobotics/ign-common/pull/215)

## Ignition Common 3.13.1 (2021-05-03)

1. Fix join paths to remove duplicate separators
    * [Pull request #205](https://github.com/ignitionrobotics/ign-common/pull/209)

1. Fix loading collada files with multiple texcoord sets using the same offset.
    * [Pull request #208](https://github.com/ignitionrobotics/ign-common/pull/208)

## Ignition Common 3.13.0 (2021-04-27)

1. Add function to convert single channel image data to RGB image
    * [Pull request #205](https://github.com/ignitionrobotics/ign-common/pull/205)

1. Avoid duplication of / in joinPaths (Windows)
    * [Pull request #201](https://github.com/ignitionrobotics/ign-common/pull/201)

1. Fix colladaLoader on Windows
    * [Pull request #200](https://github.com/ignitionrobotics/ign-common/pull/200)

1. Backport #188: Fix crash when using BVH animations.
    * [Pull request #199](https://github.com/ignitionrobotics/ign-common/pull/199)

1. Improved Windows support
    * [Pull request #197](https://github.com/ignitionrobotics/ign-common/pull/197)

## Ignition Common 3.12.0 (2021-04-06)

1. Remove use of _SOURCE and _BINARY dirs in tests.
    * [Pull request #158](https://github.com/ignitionrobotics/ign-common/pull/158)

1. Add bazel buildsystem support.
    * [Pull request #123](https://github.com/ignitionrobotics/ign-common/pull/123)

1. Use matching malloc/free for AudioDecoder.
    * [Pull request #192](https://github.com/ignitionrobotics/ign-common/pull/192)

1. Fixed tutorial link in the README.md.
    * [Pull request #194](https://github.com/ignitionrobotics/ign-common/pull/194)

1. Function to serialize compressed image to buffer
    * [Pull request #139](https://github.com/ignitionrobotics/ign-common/pull/139)

## Ignition Common 3.11.1 (2021-03-08)

1. Relax expectation so encoder test passes on ARM
    * [Pull request #183](https://github.com/ignitionrobotics/ign-common/pull/183)

1. Revert "Associate library materials effect with meshes (#151)"
    * [Pull request #182](https://github.com/ignitionrobotics/ign-common/pull/182)

## Ignition Common 3.11.0 (2021-03-05)

1. Fix image red and blue swapping
    * [Pull request #162](https://github.com/ignitionrobotics/ign-common/pull/162)

1. Remove issue & PR templates
    * [Pull request #174](https://github.com/ignitionrobotics/ign-common/pull/174)

1. Prevent console spamming when exporting a mesh that has multiple texture coordinates
    * [Pull request #171](https://github.com/ignitionrobotics/ign-common/pull/171)

1. Associate library materials effect with meshes
    * [Pull request #151](https://github.com/ignitionrobotics/ign-common/pull/151)
    * Reverted in 3.11.1

1. Added issue & PR templates
    * [Pull request #166](https://github.com/ignitionrobotics/ign-common/pull/166)

1. Added support for HW-accelerated video encoding.
    * [Pull request #125](https://github.com/ignitionrobotics/ign-common/pull/125)
    * [Pull request #169](https://github.com/ignitionrobotics/ign-common/pull/169)
    * [Pull request #175](https://github.com/ignitionrobotics/ign-common/pull/175)

1. Added FlagSet utility class
    * [Pull request #118](https://github.com/ignitionrobotics/ign-common/pull/118)

1. Disable failing VideoEncoder and Audioecoder tests on Windows
    * [Pull request #149](https://github.com/ignitionrobotics/ign-common/pull/149)

1. Added ellipsoid mesh
    * [Pull request #154](https://github.com/ignitionrobotics/ign-common/pull/154)
    * [Pull request #159](https://github.com/ignitionrobotics/ign-common/pull/159)

1. Added capsule mesh
    * [Pull request #155](https://github.com/ignitionrobotics/ign-common/pull/155)

## Ignition Common 3.10.1 (2021-01-20)

1. Fix transparency tag in the Collada exporter.
    * [Pull Request 152](https://github.com/ignitionrobotics/ign-common/pull/152)

## Ignition Common 3.10.0 (2021-01-05)

1. Added render order to material class
    * [Pull Request 142](https://github.com/ignitionrobotics/ign-common/pull/142)

1. Fix memory leak in Pbr.cc
    * [Pull Request 147](https://github.com/ignitionrobotics/ign-common/pull/147)

## Ignition Common 3.9.0 (2020-12-21)

1. Support multiple texture coordinate sets
    * [Pull Request 131](https://github.com/ignitionrobotics/ign-common/pull/131)

1. Add Lightmap to material
    * [Pull Request 132](https://github.com/ignitionrobotics/ign-common/pull/132)

## Ignition Common 3.8.0 (2020-12-08)

1. Call ignition::common::load() from Video constructor
    * [Pull Request 111](https://github.com/ignitionrobotics/ign-common/pull/111)

1. Cleanup
    * Drop unused forward declaration: [Pull Request 113](https://github.com/ignitionrobotics/ign-common/pull/113)
    * Remove two known flaky tests from Windows CI: [Pull Request 119](https://github.com/ignitionrobotics/ign-common/pull/119)
    * Make ignstrtok private to Util.cc: [Pull Request 134](https://github.com/ignitionrobotics/ign-common/pull/134)
    * Fix memory leaks [Pull Request 136](https://github.com/ignitionrobotics/ign-common/pull/136)

1. Windows
    * Do not hardcode USE_EXTERNAL_TINYXML2 to OFF: [Pull Request 116](https://github.com/ignitionrobotics/ign-common/pull/116)
    * Fix IGN_HOMEDIR on Windows: [Pull Request 127](https://github.com/ignitionrobotics/ign-common/pull/127)
    * Improve Windows support: [Pull Request 128](https://github.com/ignitionrobotics/ign-common/pull/128)
    * Fix Windows AV CI warnings and test failures: [Pull Request 135](https://github.com/ignitionrobotics/ign-common/pull/135)

1. Fix video encoder timing
    * [Pull Request 105](https://github.com/ignitionrobotics/ign-common/pull/105)

1. Feature: Add transformation matrix to ColladaExport
    * [Pull Request 100](https://github.com/ignitionrobotics/ign-common/pull/100)
    * Fix: [Pull Request 133](https://github.com/ignitionrobotics/ign-common/pull/133)

## Ignition Common 3.7.0 (2020-10-21)

1. Miscellaneous Cleanups
    * Feature: Join function for combining strings with a delimiter
    * [Pull Request 107](https://github.com/ignitionrobotics/ign-common/pull/107)

1. Improve fork experience
    * [Pull Request 103](https://github.com/ignitionrobotics/ign-common/pull/103)

1. Add Console example
    * [Pull Request 106](https://github.com/ignitionrobotics/ign-common/pull/106)

1. Support fuel URLs for textures
    * [Pull Request 102](https://github.com/ignitionrobotics/ign-common/pull/102)

1. Fix loading dae file with hierarchical node that does not have a name
    * [Pull Request 101](https://github.com/ignitionrobotics/ign-common/pull/101)

1. Fix more Animation memory leaks
    * [Pull Request 98](https://github.com/ignitionrobotics/ign-common/pull/98)

1. Fix env behavior to return true on empty vars
    * Feature: Add setenv for setting environment variables
    * Feature: Add unsetenv for clearing environment variables
    * Feature: Add alternative env implementation that respects empty variables
    * [Pull Request 97](https://github.com/ignitionrobotics/ign-common/pull/97)

## Ignition Common 3.6.1 (2020-09-16)

1. Fix hierarchical submesh processing in the ColladaLoader.
    * [Pull Request 78](https://github.com/ignitionrobotics/ign-common/pull/78)

1. Tutorial updates
    * [Pull Request 86](https://github.com/ignitionrobotics/ign-common/pull/86)
    * [Pull Request 87](https://github.com/ignitionrobotics/ign-common/pull/87)
    * [Pull Request 92](https://github.com/ignitionrobotics/ign-common/pull/92)

1. Fix the AudioDecoder class
    * [Pull Request 81](https://github.com/ignitionrobotics/ign-common/pull/81)

1. Fix actions CI
    * [Pull Request 83](https://github.com/ignitionrobotics/ign-common/pull/83)

1. Fix trajectory info memory leak
    * [Pull Request 93](https://github.com/ignitionrobotics/ign-common/pull/93)

1. Remove URI warning on constructor
    * [Pull Request 94](https://github.com/ignitionrobotics/ign-common/pull/94)

## Ignition Common 3.6.0 (2020-06-09)

1. Refactor Image::MaxColor to reduce computation time
    * [Pull request 66](https://github.com/ignitionrobotics/ign-common/pull/66)

1. Update to gtest 1.10.0 with patch for gcc warnings
    * [Pull request 67](https://github.com/ignitionrobotics/ign-common/pull/67)

1. Fix ColladaLoader loading <transparent> tag when <transparency> does not exist
    * [Pull request 68](https://github.com/ignitionrobotics/ign-common/pull/68)

1. Remove unused included headers
    * [Pull request 71](https://github.com/ignitionrobotics/ign-common/pull/71)

1. More file path APIs
    * [Pull request 70](https://github.com/ignitionrobotics/ign-common/pull/70)

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
