# ruff: noqa: D205

from __future__ import annotations

from ._bound_function import BoundFunction
from ._uenum import UEnum
from ._uobject import UObject, notify_changes
from ._uobject_children import (
    UArrayProperty,
    UBlueprintGeneratedClass,
    UBoolProperty,
    UByteAttributeProperty,
    UByteProperty,
    UClass,
    UClassProperty,
    UComponentProperty,
    UConst,
    UDelegateProperty,
    UDoubleProperty,
    UEnumProperty,
    UField,
    UFloatAttributeProperty,
    UFloatProperty,
    UFunction,
    UInt8Property,
    UInt16Property,
    UInt64Property,
    UIntAttributeProperty,
    UInterfaceProperty,
    UIntProperty,
    ULazyObjectProperty,
    UMulticastDelegateProperty,
    UNameProperty,
    UObjectProperty,
    UProperty,
    UScriptStruct,
    USoftClassProperty,
    USoftObjectProperty,
    UStrProperty,
    UStruct,
    UStructProperty,
    UTextProperty,
    UUInt16Property,
    UUInt32Property,
    UUInt64Property,
    UWeakObjectProperty,
)
from ._weak_pointer import WeakPointer
from ._wrapped_array import WrappedArray
from ._wrapped_multicast_delegate import WrappedMulticastDelegate
from ._wrapped_struct import IGNORE_STRUCT, WrappedStruct

__all__: tuple[str, ...] = (
    "IGNORE_STRUCT",
    "BoundFunction",
    "UArrayProperty",
    "UBlueprintGeneratedClass",
    "UBoolProperty",
    "UByteAttributeProperty",
    "UByteProperty",
    "UClass",
    "UClassProperty",
    "UComponentProperty",
    "UConst",
    "UDelegateProperty",
    "UDoubleProperty",
    "UEnum",
    "UEnumProperty",
    "UField",
    "UFloatAttributeProperty",
    "UFloatProperty",
    "UFunction",
    "UInt8Property",
    "UInt16Property",
    "UInt64Property",
    "UIntAttributeProperty",
    "UIntProperty",
    "UInterfaceProperty",
    "ULazyObjectProperty",
    "UMulticastDelegateProperty",
    "UNameProperty",
    "UObject",
    "UObjectProperty",
    "UProperty",
    "UScriptStruct",
    "USoftClassProperty",
    "USoftObjectProperty",
    "UStrProperty",
    "UStruct",
    "UStructProperty",
    "UTextProperty",
    "UUInt16Property",
    "UUInt32Property",
    "UUInt64Property",
    "UWeakObjectProperty",
    "WeakPointer",
    "WrappedArray",
    "WrappedMulticastDelegate",
    "WrappedStruct",
    "dir_includes_unreal",
    "notify_changes",
)

def dir_includes_unreal(should_include: bool) -> None:
    """
    Sets if `__dir__` should include dynamic unreal properties, specific to the
    object. Defaults to true.

    Args:
        should_include: True if to include dynamic properties, false to not.
    """
