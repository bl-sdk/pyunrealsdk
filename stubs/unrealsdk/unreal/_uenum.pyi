from __future__ import annotations

from enum import IntFlag

from ._uobject_children import UField

class UEnum(UField):
    def _as_py(self) -> type[IntFlag]:
        """
        Generates a compatible IntFlag enum.

        Returns:
            An IntFlag enum compatible with this enum.
        """
