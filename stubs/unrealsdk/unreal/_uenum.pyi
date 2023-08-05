from __future__ import annotations

from enum import IntFlag, EnumMeta

from ._uobject_children import UField

class _UnrealEnumMeta(EnumMeta):
    def __getattr__(self, name: str) -> IntFlag: ...

class _UnrealEnum(IntFlag, metaclass=_UnrealEnumMeta):
    _unreal: UEnum

class UEnum(UField):
    def _as_py(self) -> type[_UnrealEnum]:
        """
        Generates a compatible IntFlag enum.

        Returns:
            An IntFlag enum compatible with this enum.
        """
