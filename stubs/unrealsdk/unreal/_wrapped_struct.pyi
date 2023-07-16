from __future__ import annotations

from typing import Any, overload

from ._uobject_children import UField, UScriptStruct, UStruct

class WrappedStruct:
    """
    An unreal struct wrapper.
    """

    _type: UStruct

    def __dir__(self) -> list[str]:
        """
        Gets the attributes which exist on this struct.

        Includes both python attributes and unreal fields. This can be changed to only
        python attributes by calling dir_includes_unreal.

        Returns:
            A list of attributes which exist on this object.
        """
    @overload
    def __getattr__(self, name: str) -> Any:
        """
        Reads an unreal field off of the struct.

        Automatically looks up the relevant UField.

        Args:
            name: The name of the field to get.
        Returns:
            The field's value.
        """
    @overload
    def __getattr__(self, field: UField) -> Any:  # type: ignore[misc]  # invalid signature for getattr
        """
        Reads an unreal field off of the struct.

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
    def __init__(self, type: UScriptStruct, /, *args: Any, **kwargs: Any) -> None:
        """
        Creates a new wrapped struct.

        Args:
            type: The type of struct to create.
            *args, **kwargs: Fields on the struct to initialize.
        """
    def __repr__(self) -> str:
        """
        Gets a string representation of this struct.

        Returns:
            The string representation.
        """
    @overload
    def __setattr__(self, name: str, value: Any) -> None:
        """
        Writes a value to an unreal field on the struct.

        Automatically looks up the relevant UField.

        Args:
            name: The name of the field to set.
            value: The value to write.
        """
    @overload
    def __setattr__(self, field: UField, value: Any) -> None:  # type: ignore[misc]  # invalid signature for getattr
        """
        Writes a value to an unreal field on the struct.

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
