# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

## Ignition Common 3.X to 4.X

### Modifications

1. Corrected `BAYER_RGGR8` to `BAYER_BGGR8` in `PixelFormatName` and
   `PixelFormatType` located in `graphics/include/ignition/common/Image.hh`.

1. URI parsing has updated to follow the specification more closely.
    * An empty URI Path is valid.
    * Double forward slashes (//) are valid in a URI Path.
    * A URI Query does not require the `key=value` format. For example
    a valid query can be "?aquery" and even "?aquery?".
    * A URI authority is optional. If present, then a URI authority begins
    with two forward slashes and immediately follows the URI scheme. A host
    must be present if an authority is present. All this means the following
    is invalid, `file:///abs/path/to/file`. The proper form is
    `file:/abs/path/to/file`.

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

