# Changelog

## Upcoming

- Fixed weak pointer type hinting to allow for null pointers. This always worked at runtime.

  [1cbded47](https://github.com/bl-sdk/pyunrealsdk/commit/1cbded47)
  
- Added support for Delegate and Multicast Delegate properties.

  [04d47f92](https://github.com/bl-sdk/pyunrealsdk/commit/04d47f92),
  [2876f098](https://github.com/bl-sdk/pyunrealsdk/commit/2876f098)

- Added a repr to `BoundFunction`, as these are now returned by delegates.

  [22082579](https://github.com/bl-sdk/pyunrealsdk/commit/22082579)

## v1.3.0

Also see the unrealsdk v1.3.0 changelog [here](https://github.com/bl-sdk/unrealsdk/blob/master/changelog.md#v130).

- Added bindings for the new classes introduced in unrealsdk v1.3.0 - `WeakPointer`,
  `ULazyObjectProperty`, `USoftObjectProperty`, and `USoftClassProperty`

  [18294df4](https://github.com/bl-sdk/pyunrealsdk/commit/18294df4),
  [7e724f1e](https://github.com/bl-sdk/pyunrealsdk/commit/7e724f1e),
  [6558e1cc](https://github.com/bl-sdk/pyunrealsdk/commit/6558e1cc)

- Fixed that hooks could not always be removed after adding, or that they might not always fire.

  [unrealsdk@227a93d2](https://github.com/bl-sdk/unrealsdk/commit/227a93d2)

## v1.2.0

Also see the unrealsdk v1.2.0 changelog [here](https://github.com/bl-sdk/unrealsdk/blob/master/changelog.md#v120).

- Added bindings for the new classes introduced in unrealsdk v1.2.0 - `UByteAttributeProperty`,
  `UComponentProperty`, `UFloatAttributeProperty`, `UIntAttributeProperty`, and
  `UByteProperty::Enum`.

  [ab211486](https://github.com/bl-sdk/pyunrealsdk/commit/ab211486)

- Getting a byte property which has an associated enum will now return the appropriate Python enum,
  in the same way as an enum property does. Byte properties without an enum still return an int.

  [ab211486](https://github.com/bl-sdk/pyunrealsdk/commit/ab211486)

- Fixed that it was impossible to set Python properties on unreal objects.

  [8b75fbbf](https://github.com/bl-sdk/pyunrealsdk/commit/8b75fbbf),
  [730a813f](https://github.com/bl-sdk/pyunrealsdk/commit/730a813f)

- Changed the log level specific printers, `unrealsdk.logging.error` et al., to each use their own,
  logger objects rather than modifying `sys.stdout` in place.

  [285e276a](https://github.com/bl-sdk/pyunrealsdk/commit/285e276a),
  [34e190b6](https://github.com/bl-sdk/pyunrealsdk/commit/34e190b6)

- Added the `PYUNREALSDK_PYEXEC_ROOT` env var to let you customize where `pyexec` commands are run
  relative to.

  [fdbda407](https://github.com/bl-sdk/pyunrealsdk/commit/fdbda407)

- Fixed that a fully qualified `unrealsdk.find_class` would not allow subclasses. This was most
  notable with blueprint generated classes.

  [unrealsdk@643fb46e](https://github.com/bl-sdk/unrealsdk/commit/643fb46e)

- Updated various docstrings and type stubs to be more accurately.

  [d66295ef](https://github.com/bl-sdk/pyunrealsdk/commit/d66295ef),
  [0df05cea](https://github.com/bl-sdk/pyunrealsdk/commit/0df05cea),
  [285e276a](https://github.com/bl-sdk/pyunrealsdk/commit/285e276a)

- Restructured CMake to allow you to define the Python version to link against directly within it,
  similarly to unrealsdk.

  ```cmake
  set(UNREALSDK_ARCH x64)
  set(UNREALSDK_UE_VERSION UE4)
  set(EXPLICIT_PYTHON_ARCH win64)
  set(EXPLICIT_PYTHON_VERSION 3.12.3)

  add_subdirectory(libs/pyunrealsdk)
  ```
  [abca72b3](https://github.com/bl-sdk/pyunrealsdk/commit/abca72b3)

- Release the GIL during unreal function calls, to try avoid a deadlock when running with
  `UNREALSDK_LOCKING_PROCESS_EVENT`.

  [31fdb4ee](https://github.com/bl-sdk/pyunrealsdk/commit/31fdb4ee)

- Upgraded pybind.

  [b1335304](https://github.com/bl-sdk/pyunrealsdk/commit/b1335304)

## v1.1.1
- Updated CI and stubs to Python 3.12

  [0d0cbce9](https://github.com/bl-sdk/pyunrealsdk/commit/0d0cbce9)

## v1.1.0

Also see the unrealsdk v1.1.0 changelog [here](https://github.com/bl-sdk/unrealsdk/blob/master/changelog.md#v110).

- Add support for [debugpy](https://github.com/microsoft/debugpy)., to let it trigger python
  breakpoints on other threads. See the new [Debugging](Readme.md#Debugging) section in the readme
  for more.

  [082f1252](https://github.com/bl-sdk/pyunrealsdk/commit/082f1252)

- Add bindings for:
  - `unrealsdk.load_package`

    [82e56fe4](https://github.com/bl-sdk/pyunrealsdk/commit/82e56fe4)

  - `UObject._path_name` - which does not include the class, in contrast to `__repr__`.

    [363fbe48](https://github.com/bl-sdk/pyunrealsdk/commit/363fbe48)

  - `UStruct._superfields`

    [2f5dca2d](https://github.com/bl-sdk/pyunrealsdk/commit/2f5dca2d)


- Add const getters to `StaticPyObject`.

  [8edc4580](https://github.com/bl-sdk/pyunrealsdk/commit/8edc4580)

- Add support for building using standard GCC-based MinGW. This is not tested in CI however, as it
  requires a newer version than that available in Github Actions.

  [d692f0e9](https://github.com/bl-sdk/pyunrealsdk/commit/d692f0e9),
  [2727b17c](https://github.com/bl-sdk/pyunrealsdk/commit/2727b17c)

- Update stub linting, and do a few minor fixups.

  [84aedc62](https://github.com/bl-sdk/pyunrealsdk/commit/84aedc62),
  [a228b56e](https://github.com/bl-sdk/pyunrealsdk/commit/a228b56e)


## v1.0.0
- Initial Release
