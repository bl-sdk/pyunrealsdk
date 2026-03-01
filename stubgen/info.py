from __future__ import annotations

import textwrap
from dataclasses import dataclass, field
from enum import Enum, auto
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Iterator

__all__: tuple[str, ...] = (
    "ArgInfo",
    "AttrInfo",
    "ClassInfo",
    "EnumInfo",
    "FuncInfo",
    "FuncType",
    "InfoDict",
    "ModuleInfo",
)


type InfoDict = dict[str, AttrInfo | ClassInfo | EnumInfo | FuncInfo | ModuleInfo]


def format_docstring(docstring: str, indent: str = "") -> str:
    """
    Formats a docstring.

    Args:
        docstring: The raw docstring.
        indent: How much to indent the docstring.
    Returns:
        The formatted docstring.
    """
    if "\n" not in docstring:
        return textwrap.indent(f'"""{docstring}"""', indent)

    output = textwrap.indent(f'"""\n{docstring}"""', indent)

    if docstring.splitlines()[1]:
        # 1 blank line required between summary line and description
        output += " # noqa: D205"

    return output


def format_deprecation_message(msg: str, indent: str = "") -> str:
    """
    Formats a deprecation message, including the decorator + newline.

    Args:
        msg: The message to format.
        indent: How much to indent the deprecation message.
    Returns:
        The formatted message.
    """
    single_line = f'{indent}@warnings.deprecated("{msg}")'
    if len(single_line) < 100:  # noqa: PLR2004
        return single_line + "\n"

    wrapped = textwrap.wrap(msg, 100 - 2 - 4 - len(indent), drop_whitespace=False)
    return textwrap.indent(
        "@warnings.deprecated(" + "".join(f'\n{indent}    "{line}"' for line in wrapped) + ",\n)\n",
        indent,
    )


@dataclass
class ModuleInfo:
    name: str
    docstring: str | None = None

    def declare(self) -> str:
        """
        Writes the docstring for this module.

        Returns:
            A string holding the module docstring.
        """
        if self.docstring:
            return format_docstring(self.docstring)
        return ""


@dataclass
class AttrInfo:
    name: str
    type_hint: str
    readonly_prop: bool = False

    def declare(self) -> str:
        """
        Declares this attribute.

        Returns:
            A string holding the declaration.
        """
        if self.readonly_prop:
            output = f"@property\ndef {self.name}(self) -> {self.type_hint}: ..."
            if not self.name.islower():
                # Allow uppercase names - we treat unreal attributes as uppercase
                output += "  # noqa: N802"
            return output
        return f"{self.name}: {self.type_hint}"


@dataclass
class ArgInfo:
    name: str
    type_hint: str | None  # Should only be none on self/cls and the pos-only/kw-only markers
    default: str | None

    def declare(self) -> str:
        """
        Declares this arg.

        Returns:
            A string holding the declaration.
        """
        output = self.name
        if self.type_hint:
            output += f": {self.type_hint}"
        if self.default:
            output += f" = {self.default}"
        return output


class FuncType(Enum):
    Func = auto()
    Method = auto()
    StaticMethod = auto()
    ClassMethod = auto()


@dataclass
class FuncInfo:
    func_type: FuncType
    name: str
    ret: str
    args: list[ArgInfo] = field(default_factory=list)
    docstring: str | None = None
    deprecated: str | None = None
    generic: str | None = None

    # None on the overloads themselves to avoid nesting.
    overloads: list[FuncInfo] | None = field(default_factory=list)

    def declare(self) -> str:
        """
        Declares this function.

        Returns:
            A string holding the declaration.
        """
        output = ""
        for overload in self.overloads or ():
            output += "@overload\n"
            output += overload.declare()

        if self.deprecated:
            output += format_deprecation_message(self.deprecated)
        if self.func_type == FuncType.StaticMethod:
            output += "@staticmethod\n"
        elif self.func_type == FuncType.ClassMethod:
            output += "@classmethod\n"

        output += f"def {self.name}{self.generic or ''}"
        output += f"({', '.join(a.declare() for a in self.args)}) -> {self.ret}:\n"

        if self.docstring:
            output += format_docstring(self.docstring, "    ")
            output += "\n"
        else:
            output += "    ...\n"

        return output


@dataclass
class ClassInfo:
    name: str
    super_class: str | None
    docstring: str | None = None
    attrs: list[AttrInfo] = field(default_factory=list)
    methods: list[FuncInfo] = field(default_factory=list)
    deprecated: str | None = None
    generic: str | None = None

    def declare(self) -> str:
        """
        Declares this class.

        Returns:
            A string holding the declaration.
        """
        output = ""
        if self.deprecated:
            output += format_deprecation_message(self.deprecated)
        output += f"class {self.name}{self.generic or ''}"
        if self.super_class is not None:
            output += f"({self.super_class})"
        output += ":\n"

        if self.docstring:
            output += format_docstring(self.docstring, "    ")
            output += "\n"

        for attr in self.attrs:
            output += textwrap.indent(attr.declare(), "    ") + "\n"
        if self.attrs:
            output += "\n"

        if self.methods:
            for method in self.iter_methods():
                output += textwrap.indent(method.declare(), "    ")

        if not any((self.docstring, self.attrs, self.methods)):
            output += "    ...\n"

        return output

    def iter_methods(self) -> Iterator[FuncInfo]:
        """
        Iterates through all methods on this class, in the order we want to render them.

        Yields:
            Ordeded methods under this class.
        """
        init: FuncInfo | None = None
        new: FuncInfo | None = None
        magic_methods: list[FuncInfo] = []
        standard_methods: list[FuncInfo] = []
        for method in self.methods:
            name = method.name
            if name == "__init__":
                init = method
            elif name == "__new__":
                new = method
            elif name.startswith("__") and name.endswith("__"):
                magic_methods.append(method)
            else:
                standard_methods.append(method)

        if init:
            yield init
        if new:
            yield new
        yield from sorted(magic_methods, key=lambda m: m.name)
        yield from sorted(standard_methods, key=lambda m: m.name)


@dataclass
class EnumValueInfo:
    name: str
    docstring: str | None = None

    def declare(self) -> str:
        output = f"{self.name} = ..."
        if self.docstring:
            output += "\n"
            output += format_docstring(self.docstring)

        return output


@dataclass
class EnumInfo:
    name: str
    values: list[EnumValueInfo] = field(default_factory=list)
    docstring: str | None = None

    def declare(self) -> str:
        """
        Declares this enum.

        Returns:
            A string holding the declaration.
        """
        output = f"class {self.name}(Enum):\n"
        if self.docstring:
            output += format_docstring(self.docstring, "    ")
            output += "\n"

        output += "\n"
        for val in self.values:
            output += textwrap.indent(val.declare(), "    ") + "\n"

        return output
