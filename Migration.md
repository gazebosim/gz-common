# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

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

