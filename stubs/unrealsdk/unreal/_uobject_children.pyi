from __future__ import annotations

from collections.abc import Iterator
from typing import Any

from ._uobject import UObject
from ._wrapped_struct import WrappedStruct

class UField(UObject):
    """
    An unreal field object.
    """

    Next: UField | None

class UProperty(UField):
    """
    The base class of all unreal properties.
    """

    ArrayDim: int
    ElementSize: int
    Offset_Internal: int
    PropertyFlags: int
    PropertyLinkNext: UProperty | None

class UStruct(UField):
    """
    An unreal struct object.
    """

    Children: UField | None
    PropertyLink: UProperty | None
    SuperField: UStruct | None

    def _fields(self) -> Iterator[UField]:
        """
        Iterates over all fields in the struct.

        Returns:
            An iterator over all fields in the struct.
        """
    def _find(self, name: str) -> UField:
        """
        Finds a child field by name.

        Throws an exception if the child is not found.

        Args:
            name: The name of the child field.
        Returns:
            The found child field.
        """
    def _find_prop(self, name: str) -> UProperty:
        """
        Finds a child property by name.

        When known to be a property, this is more efficient than _find.

        Throws an exception if the child is not found.

        Args:
            name: The name of the child property.
        Returns:
            The found child property.
        """
    def _get_struct_size(self) -> int:
        """
        Gets the actual size of the described structure, including alignment.

        Returns:
            The size which must be allocated.
        """
    def _inherits(self, base_struct: UStruct) -> bool:
        """
        Checks if this structs inherits from another.

        Also returns true if this struct *is* the given struct.

        Args:
            base_struct: The base struct to check if this inherits from.
        Returns:
            True if this struct is the given struct, or inherits from it.
        """
    def _properties(self) -> Iterator[UProperty]:
        """
        Iterates over all properties in the struct.

        Returns:
            An iterator over all properties in the struct.
        """

class UObjectProperty(UProperty):
    """
    An unreal object property object.
    """

    @property
    def PropertyClass(self) -> UClass: ...

class UDoubleProperty(UProperty):
    """
    An unreal double property object.
    """

class UArrayProperty(UProperty):
    """
    An unreal array property object.
    """

    @property
    def Inner(self) -> UProperty: ...

class UFloatProperty(UProperty):
    """
    An unreal float property object.
    """

class UFunction(UStruct):
    """
    An unreal function object.
    """

    FunctionFlags: int
    NumParams: int
    ParamsSize: int
    ReturnValueOffset: int

    def _find_return_param(self) -> UProperty | None:
        """
        Finds the return param for this function (if it exists).

        Returns:
            The return param, or None if it doesn't exist.
        """

class UInt16Property(UProperty):
    """
    An unreal int16 property object.
    """

class UInt64Property(UProperty):
    """
    An unreal int64 property object.
    """

class UInt8Property(UProperty):
    """
    An unreal int8 property object.
    """

class UIntProperty(UProperty):
    """
    An unreal int32 property object.
    """

class UInterfaceProperty(UProperty):
    """
    An unreal interface property object.
    """

    @property
    def InterfaceClass(self) -> UClass: ...

class UNameProperty(UProperty):
    """
    An unreal name property object.
    """

class UBoolProperty(UProperty):
    """
    An unreal bool property object.
    """

    @property
    def FieldMask(self) -> int: ...

class UClassProperty(UObjectProperty):
    """
    An unreal class property object.
    """

    @property
    def MetaClass(self) -> UClass: ...

class UByteProperty(UProperty):
    """
    An unreal uint8 property object.
    """

class UScriptStruct(UStruct):
    """
    An unreal script struct object.
    """

    StructFlags: int

    def __call__(self, *args: Any, **kwargs: Any) -> WrappedStruct:
        """
        Helper to create a new wrapped struct using this type.

        Args:
            *args, **kwargs: Fields on the struct to initialize.
        Returns:
            A new WrappedStruct.
        """

class UStrProperty(UProperty):
    """
    An unreal string property object.
    """

class UClass(UStruct):
    """
    An unreal class object.
    """

    ClassDefaultObject: UObject

    def _implements(self, interface: UClass) -> bool:
        """
        Checks if this class implements a given interface.

        Args:
            interface: The interface to check.
        Returns:
            True if this class implements the interface, false otherwise.
        """

class UStructProperty(UProperty):
    """
    An unreal struct property object.
    """

    @property
    def Struct(self) -> UScriptStruct: ...

class UUInt16Property(UProperty):
    """
    An unreal uint16 property object.
    """

class UUInt32Property(UProperty):
    """
    An unreal uint32 property object.
    """

class UUInt64Property(UProperty):
    """
    An unreal uint64 property object.
    """
