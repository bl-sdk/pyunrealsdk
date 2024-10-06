from __future__ import annotations

from typing import Any

from ._uobject_children import UField, UFunction, UScriptStruct, UStruct

class WrappedStruct:
    _type: UStruct

    def __dir__(self) -> list[str]:
        """
        Gets the attributes which exist on this struct.

        Includes both python attributes and unreal fields. This can be changed to only
        python attributes by calling dir_includes_unreal.

        Returns:
            A list of attributes which exist on this object.
        """
    def __getattr__(self, name: str) -> Any:
        """
        Reads an unreal field off of the struct.

        Automatically looks up the relevant UField.

        Args:
            name: The name of the field to get.
        Returns:
            The field's value.
        """
    def __init__(self, type: UScriptStruct | UFunction, /, *args: Any, **kwargs: Any) -> None:
        """
        Creates a new wrapped struct.

        Args:
            type: The type of struct to create.
            *args: Fields on the struct to initialize.
            **kwargs: Fields on the struct to initialize.
        """
    def __repr__(self) -> str:
        """
        Gets a string representation of this struct.

        Returns:
            The string representation.
        """
    def __setattr__(self, name: str, value: Any) -> None:
        """
        Writes a value to an unreal field on the struct.

        Automatically looks up the relevant UField.

        Args:
            name: The name of the field to set.
            value: The value to write.
        """
    def _get_field(self, field: UField) -> Any:
        """
        Reads an unreal field off of the struct.

        In performance critical situations, rather than use __getattr__, you can look up
        the UField beforehand (via struct._type._find()), then pass it directly to this
        function. This does not get validated, passing a field which doesn't exist on
        the struct is undefined behaviour.

        Args:
            field: The field to get.
        Returns:
            The field's value.
        """
    def _set_field(self, field: UField, value: Any) -> None:
        """
        Writes a value to an unreal field on the struct.

        In performance critical situations, rather than use __setattr__, you can look up
        the UField beforehand (via struct._type._find()), then pass it directly to this
        function. This does not get validated, passing a field which doesn't exist on
        the struct is undefined behaviour.

        Args:
            field: The field to set.
            value: The value to write.
        """
