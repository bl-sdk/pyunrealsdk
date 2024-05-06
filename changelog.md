# Changelog

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
