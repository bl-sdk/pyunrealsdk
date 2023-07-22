from __future__ import annotations

from collections.abc import Iterator
from typing import Any

from ._uenum import UEnum
from ._uobject import UObject
from ._wrapped_struct import WrappedStruct

class UField(UObject):
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
    @property
    def PropertyClass(self) -> UClass: ...

class UDoubleProperty(UProperty): ...

class UArrayProperty(UProperty):
    @property
    def Inner(self) -> UProperty: ...

class UFloatProperty(UProperty): ...

class UFunction(UStruct):
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

class UInt16Property(UProperty): ...
class UInt64Property(UProperty): ...
class UInt8Property(UProperty): ...
class UIntProperty(UProperty): ...

class UInterfaceProperty(UProperty):
    @property
    def InterfaceClass(self) -> UClass: ...

class UNameProperty(UProperty): ...

class UBoolProperty(UProperty):
    @property
    def FieldMask(self) -> int: ...

class UClassProperty(UObjectProperty):
    @property
    def MetaClass(self) -> UClass: ...

class UByteProperty(UProperty): ...

class UScriptStruct(UStruct):
    StructFlags: int

    def __call__(self, *args: Any, **kwargs: Any) -> WrappedStruct:
        """
        Helper to create a new wrapped struct using this type.

        Args:
            *args, **kwargs: Fields on the struct to initialize.
        Returns:
            A new WrappedStruct.
        """

class UStrProperty(UProperty): ...

class UClass(UStruct):
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
    @property
    def Struct(self) -> UScriptStruct: ...

class UUInt16Property(UProperty): ...
class UUInt32Property(UProperty): ...
class UUInt64Property(UProperty): ...
class UBlueprintGeneratedClass(UClass): ...

class UConst:
    @property
    def Value(self) -> str: ...
    @Value.setter
    def Value(self, Value: str) -> None: ...

class UEnumProperty(UProperty, UField, UObject):
    @property
    def Enum(self) -> UEnum: ...
    @property
    def UnderlyingProp(self) -> UProperty: ...
