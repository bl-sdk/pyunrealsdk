from __future__ import annotations

from collections.abc import Iterator

from ._uenum import UEnum
from ._uobject import UObject
from ._wrapped_array import WrappedArray
from ._wrapped_struct import WrappedStruct

# ruff: noqa: N802, N803

# ======== First Layer Subclasses ========

class UField(UObject):
    Next: UField | None

# ======== Second Layer Subclasses ========

class UConst(UField):
    Value: str

class UProperty(UField):
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
    def _superfields(self) -> Iterator[UStruct]:
        """
        Iterates over this struct and it's superfields.

        Note this includes this struct itself.

        Returns:
            An iterator over all superfields in the struct.
        """

# ======== Third Layer Subclasses ========

class UArrayProperty(UProperty):
    @property
    def Inner(self) -> UProperty: ...

class UBoolProperty(UProperty):
    @property
    def FieldMask(self) -> int: ...

class UByteProperty(UProperty):
    @property
    def Enum(self) -> UEnum | None: ...

class UClass(UStruct):
    ClassDefaultObject: UObject

    @property
    def Interfaces(self) -> list[UClass]: ...
    def _implements(self, interface: UClass) -> bool:
        """
        Checks if this class implements a given interface.

        Args:
            interface: The interface to check.
        Returns:
            True if this class implements the interface, false otherwise.
        """

class UDelegateProperty(UProperty):
    @property
    def Signature(self) -> UFunction: ...

class UDoubleProperty(UProperty): ...

class UEnumProperty(UProperty):
    @property
    def Enum(self) -> UEnum: ...
    @property
    def UnderlyingProp(self) -> UProperty: ...

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

class UInt8Property(UProperty): ...
class UInt16Property(UProperty): ...
class UInt64Property(UProperty): ...

class UInterfaceProperty(UProperty):
    @property
    def InterfaceClass(self) -> UClass: ...

class UIntProperty(UProperty): ...

class UMulticastDelegateProperty(UProperty):
    @property
    def Signature(self) -> UFunction: ...

class UNameProperty(UProperty): ...

class UObjectProperty(UProperty):
    @property
    def PropertyClass(self) -> UClass: ...

class UScriptStruct(UStruct):
    StructFlags: int

class UStrProperty(UProperty): ...

class UStructProperty(UProperty):
    @property
    def Struct(self) -> UScriptStruct: ...

class UTextProperty(UProperty): ...
class UUInt16Property(UProperty): ...
class UUInt32Property(UProperty): ...
class UUInt64Property(UProperty): ...

# ======== Fourth Layer Subclasses ========

class UBlueprintGeneratedClass(UClass): ...

class UByteAttributeProperty(UByteProperty):
    @property
    def ModifierStackProperty(self) -> UArrayProperty | None: ...
    @property
    def OtherAttributeProperty(self) -> UByteAttributeProperty | None: ...

class UClassProperty(UObjectProperty):
    @property
    def MetaClass(self) -> UClass: ...

class UComponentProperty(UObjectProperty): ...

class UFloatAttributeProperty(UByteProperty):
    @property
    def ModifierStackProperty(self) -> UArrayProperty | None: ...
    @property
    def OtherAttributeProperty(self) -> UByteAttributeProperty | None: ...

class UIntAttributeProperty(UByteProperty):
    @property
    def ModifierStackProperty(self) -> UArrayProperty | None: ...
    @property
    def OtherAttributeProperty(self) -> UByteAttributeProperty | None: ...

class ULazyObjectProperty(UObjectProperty):
    @staticmethod
    def _get_identifier_from(
        source: UObject | WrappedStruct,
        prop: str | ULazyObjectProperty,
        idx: int = 0,
    ) -> bytes:
        """
        Gets the Guid identifier associated with a given lazy object property.

        When using standard attribute access, lazy object properties resolve directly to
        their contained object. This function can be used to get the identifier instead.

        Args:
            source: The object or struct holding the property to get.
            prop: The lazy object property, or name thereof, to get.
            idx: If this property is a fixed sized array, which index to get.
        Returns:
            The raw 16 bytes composing the property's Guid.
        """
    @staticmethod
    def _get_identifier_from_array(
        source: WrappedArray[UObject],
        idx: int = 0,
    ) -> bytes:
        """
        Gets the Guid identifier associated with a given lazy object property.

        When using standard attribute access, lazy object properties resolve directly to
        their contained object. This function can be used to get the identifier instead.

        Args:
            source: The array holding the property to get.
            idx: The index into the array to get from.
        Returns:
            The raw 16 bytes composing the property's Guid.
        """

class USoftObjectProperty(UObjectProperty):
    @staticmethod
    def _get_identifier_from(
        source: UObject | WrappedStruct,
        prop: str | USoftObjectProperty,
        idx: int = 0,
    ) -> str:
        """
        Gets the path name identifier associated with a given soft object property.

        When using standard attribute access, soft object properties resolve directly to
        their contained object. This function can be used to get the identifier instead.

        Args:
            source: The object or struct holding the property to get.
            prop: The soft object property, or name thereof, to get.
            idx: If this property is a fixed sized array, which index to get.
        Returns:
            The path name of the object the given property is looking for.
        """
    @staticmethod
    def _get_identifier_from_array(
        source: WrappedArray[UObject],
        idx: int = 0,
    ) -> str:
        """
        Gets the path name identifier associated with a given soft object property.

        When using standard attribute access, soft object properties resolve directly to
        their contained object. This function can be used to get the identifier instead.

        Args:
            source: The array holding the property to get.
            idx: The index into the array to get from.
        Returns:
            The path name of the object the given property is looking for.
        """

class UWeakObjectProperty(UObjectProperty): ...

# ======== Fifth Layer Subclasses ========

class USoftClassProperty(USoftObjectProperty): ...
