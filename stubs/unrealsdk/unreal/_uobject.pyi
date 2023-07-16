from __future__ import annotations

from typing import Any, Never, overload

from ._uobject_children import UClass, UField

class UObject:
    """
    The base class of all unreal objects.
    """

    Class: UClass
    InternalIndex: int
    Name: str
    ObjectFlags: int
    Outer: UObject

    def __dir__(self) -> list[str]:
        """
        Gets the attributes which exist on this object.

        Includes both python attributes and unreal fields. This can be changed to only
        python attributes by calling dir_includes_unreal.

        Returns:
            A list of attributes which exist on this object.
        """
    @overload
    def __getattr__(self, name: str) -> Any:
        """
        Reads an unreal field off of the object.

        Automatically looks up the relevant UField.

        Args:
            name: The name of the field to get.
        Returns:
            The field's value.
        """
    @overload
    def __getattr__(self, field: UField) -> Any:  # type: ignore[misc]  # invalid signature for getattr
        """
        Reads an unreal field off of the object.

        In performance critical situations, you can look up the UField beforehand via
        obj.Class._find("name"), then pass it directly to this function. This does not
        get validated, passing a field which doesn't exist on the object is undefined
        behaviour.

        Note that getattr() only supports string keys, when passing a field you must
        call this function directly.

        Args:
            field: The field to get.
        Returns:
            The field's value.
        """
    def __init__(self, *args: Any, **kwargs: Any) -> Never: ...
    def __new__(cls, *args: Any, **kwargs: Any) -> Never: ...
    def __repr__(self) -> str:
        """
        Gets this object's name.

        Returns:
            This object's name.
        """
    @overload
    def __setattr__(self, name: str, value: Any) -> None:
        """
        Writes a value to an unreal field on the object.

        Automatically looks up the relevant UField.

        Args:
            name: The name of the field to set.
            value: The value to write.
        """
    @overload
    def __setattr__(self, field: UField, value: Any) -> None:  # type: ignore[misc]  # invalid signature for getattr
        """
        Writes a value to an unreal field on the object.

        In performance critical situations, you can look up the UField beforehand via
        obj.Class._find("name"), then pass it directly to this function. This does not
        get validated, passing a field which doesn't exist on the object is undefined
        behaviour.

        Note that setattr() only supports string keys, when passing a field you must
        call this function directly.

        Args:
            field: The field to set.
            value: The value to write.
        """
    def _get_address(self) -> int:
        """
        Gets the address of this object, for debugging.

        Returns:
            This object's address.
        """
