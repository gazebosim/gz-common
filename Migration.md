# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

## Ignition Common 2.X to 3.X

### Additions

1. **Event.hh**
    + Added second template argument to `EventT` to disambiguate events
      with matching signatures.

1. **profiler** component that helps measure software performance.

1. **SystemPaths.hh**
    + Search paths specified in `IGN_FILE_PATH` environment variable when
      finding files.

1. **Util.hh**
    + Added `constexpr uint64_t hash64(std::string_view)` to generate a
      hash at compile time if possible.

### Modifications

1. Depends on **ignition-cmake2**
    + Ignition-common now depends on ignition-cmake2.

1. Requires c++17.

1. (New in 3.8.0) On Windows, the value of C++ macro `IGN_HOMEDIR` changed from `HOMEPATH` to `USERPROFILE`. It is usually used to read the path to the user's home from environment. The old value pointed to a path relative to the (a) current drive letter as reported by `pwd`, not the system drive letter. The new value correctly points to an environment variable that contains the full absolute path to the user's profile. If the code did not use the macro in some unexpected way, the new behavior should work either the same or even better (it would work even when the current directory is on a non-system drive). If the code relied on this value to be relative to the current drive letter, it needs to be changed to use `HOMEPATH` directly.

## Ignition Common 1.X to 2.X

### Modifications

1. Depends on **ignition-cmake1**
    + Ignition-common now depends on ignition-cmake1, which provides
      support for Component libraries.

1. Component libraries
    + Some classes have been moved from the main ignition-common library
      to component libraries. To use these features, you must
      list them in the `find_package` call and link against them.
      - av: audio and video playback and encoding
      - events: registering and handling event callbacks
      - graphics: animation, images, and triangle meshes

## Ignition Common 0.X to 1.X

### Added dependencies

1. **ignition-cmake**
    + Ignition-math now has a build dependency on ignition-cmake, which
      allows cmake scripts to be shared across all the ignition packages.

