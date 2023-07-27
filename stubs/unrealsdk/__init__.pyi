from __future__ import annotations

from collections.abc import Iterator
from typing import Any

from . import commands, hooks, logging, unreal
from .unreal import UClass, UObject, WrappedStruct

__all__: tuple[str, ...] = (
    "commands",
    "construct_object",
    "find_all",
    "find_class",
    "find_object",
    "hooks",
    "logging",
    "make_struct",
    "unreal",
)

__version__: str
__version_info__: tuple[int, int, int]

def construct_object(
    cls: UClass | str,
    outer: UObject,
    name: str = "None",
    flags: int = 0,
    template_obj: None | UObject = None,
) -> UObject:
    """
    Constructs a new object

    Args:
        cls: The class to construct, or it's name. Required. If given as the name,
             always autodetects if fully qualified - call find_class() directly if
             you need to specify.
        outer: The outer object to construct the new object under. Required.
        name: The new object's name.
        flags: Object flags to set.
        template_obj: The template object to use.
    Returns:
        The constructed object.
    """

def find_all(cls: UClass | str, exact: bool = True) -> Iterator[UObject]:
    """
    Finds all instances of a class.

    Args:
        cls: The object's class, or class name. If given as the name, always
             autodetects if fully qualified - call find_class() directly if you need
             to specify.
        exact: If true (the default), only finds exact class matches. If false, also
               matches subclasses.
    Returns:
        A list of all instances of the class.
    """

def find_class(name: str, fully_qualified: None | bool = None) -> UClass | None:
    """
    Finds a class by name.

    Args:
        name: The class name.
        fully_qualified: If the class name is fully qualified, or None (the default)
                         to autodetect.
    Returns:
        The class, or None if not found.
    """

def find_object(cls: UClass | str, name: str) -> UObject | None:
    """
    Finds an object by name.

    Args:
        cls: The object's class, or class name. If given as the name, always
             autodetects if fully qualified - call find_class() directly if you need
             to specify.
        name: The object's name.
    Returns:
        The object, or None if not found.
    """

def make_struct(name: str, fully_qualified: None | bool = None, /, **kwargs: Any) -> WrappedStruct:
    """
    Finds and constructs a WrappedStruct by name.

    Args:
        name: The struct name.
        fully_qualified: If the struct name is fully qualified, or None (the
                         default) to autodetect.
        **kwargs: Fields on the struct to initialize.
    Returns:
        The newly constructed struct.
    """
