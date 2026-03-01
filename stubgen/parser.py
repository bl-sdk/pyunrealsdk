from __future__ import annotations

import ast
import sys
from pathlib import Path
from typing import TYPE_CHECKING

from .info import (
    ArgInfo,
    AttrInfo,
    ClassInfo,
    EnumInfo,
    EnumValueInfo,
    FuncInfo,
    FuncType,
    InfoDict,
    ModuleInfo,
)
from .preprocessor import Flavour, parse_macros_from_file

if TYPE_CHECKING:
    from collections.abc import Sequence

    from .preprocessor import ArgTokens, LexToken

__all__: tuple[str, ...] = ("parse_file",)


gathered_info: InfoDict = {}
context_stack: list[ClassInfo | EnumInfo | EnumValueInfo | FuncInfo | ModuleInfo] = []


def parse_string(tokens: Sequence[LexToken]) -> str:
    """
    Parses a series of tokens which are meant to be a single string.

    Args:
        tokens: The sequence of tokens to parse.
    Returns:
        The parsed string.
    """
    assert all(t.type in {"CPP_STRING", "CPP_WS"} for t in tokens), "expected a string"
    # Stupid smart idea: Python string literals are strictly a superset of C strings, so we can
    # parse a C string just by pretending it's a Python literal
    # Have to ignore whitespace in case of unexpected indents
    return ast.literal_eval("".join(t.value for t in tokens if t.type != "CPP_WS"))


def _create_module(full_name: str) -> None:
    module: ModuleInfo
    if (existing := gathered_info.get(full_name)) is not None:
        assert isinstance(existing, ModuleInfo), f"module has same name as existing {existing}"
        module = existing
    else:
        module = ModuleInfo(full_name)
        gathered_info[full_name] = module

    context_stack[:] = [module]


def parse_module(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_MODULE macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 1, "expected one arg"
    name = parse_string(args[0])
    _create_module(name)


def parse_submodule(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_SUBMODULE macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    outer = parse_string(args[0])
    name = parse_string(args[1])
    _create_module(outer + "." + name)


def parse_docstring(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_DOCSTRING macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 1, "expected one arg"
    docstring = parse_string(args[0])

    if "\n" in docstring:
        assert docstring[-1] == "\n", "expected multiline docstring to end with a newline"

    for line in docstring.splitlines():
        assert len(line) <= 80, f"expect all docstrings be max 80 chars in width, got:\n{line}"  # noqa: PLR2004

    assert isinstance(
        context_stack[-1],
        ClassInfo | EnumInfo | EnumValueInfo | FuncInfo | ModuleInfo,
    ), "can only add docstrings to a class, enum, function, or module"
    assert context_stack[-1].docstring is None, (
        "tried to add docstring to object that already has one"
    )
    context_stack[-1].docstring = docstring


def parse_module_attr(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_ATTR macro at module level.

    Args:
        args: The macro's args.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    name = parse_string(args[0])
    type_hint = parse_string(args[1])

    while not isinstance(context_stack[-1], ModuleInfo):
        context_stack.pop()

    full_name = ".".join(x.name for x in context_stack) + "." + name

    assert full_name not in gathered_info, f"got duplicate attribute {full_name}"
    gathered_info[full_name] = AttrInfo(name, type_hint)


def parse_class_attr(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_ATTR macro inside a class.

    Args:
        args: The macro's args.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    name = parse_string(args[0])
    type_hint = parse_string(args[1])

    assert isinstance(context_stack[-1], ClassInfo)  # guaranteed by caller
    context_stack[-1].attrs.append(AttrInfo(name, type_hint))


def parse_enum_attr(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_ATTR macro inside an enum.

    Args:
        args: The macro's args.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    name = parse_string(args[0])
    assert len(args[1]) == 0, "expected second arg to be empty"

    while isinstance(context_stack[-1], EnumValueInfo):
        context_stack.pop()
    assert isinstance(context_stack[-1], EnumInfo)  # guaranteed by caller

    value = EnumValueInfo(name)
    context_stack[-1].values.append(value)
    context_stack.append(value)


def parse_readonly_prop(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_READONLY_PROP macro inside a class.

    Args:
        args: The macro's args.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    name = parse_string(args[0])
    type_hint = parse_string(args[1])

    while isinstance(context_stack[-1], FuncInfo):
        context_stack.pop()

    assert isinstance(context_stack[-1], ClassInfo), "got a property outside of a class"
    context_stack[-1].attrs.append(AttrInfo(name, type_hint, readonly_prop=True))


def _create_func(func_type: FuncType, name: str, ret: str) -> FuncInfo:
    func = FuncInfo(func_type, name, ret)
    match func_type:
        case FuncType.Func:
            pass
        case FuncType.Method:
            # https://docs.python.org/3/reference/datamodel.html#object.__new__
            # >  __new__() is a static method (special-cased so you need not declare it as such)
            # Treat __new__ as a regular method, to avoid the decorator, just rename the first arg
            func.args.append(ArgInfo("cls" if name == "__new__" else "self", None, None))
        case FuncType.StaticMethod:
            pass
        case FuncType.ClassMethod:
            func.args.append(ArgInfo("cls", None, None))

    return func


def parse_func(args: Sequence[ArgTokens], func_type: FuncType) -> None:
    """
    Parses one of the various function stubgen macros.

    Args:
        args: The macro's args.
        func_type: What type of function is being parsed.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    name = parse_string(args[0])
    ret = parse_string(args[1])

    outer_type = ModuleInfo if func_type is FuncType.Func else ClassInfo
    while not isinstance(context_stack[-1], outer_type):
        context_stack.pop()

    func = _create_func(func_type, name, ret)

    if func_type == FuncType.Func:
        full_name = ".".join(x.name for x in context_stack) + "." + name
        assert full_name not in gathered_info, f"got duplicate function {full_name}"
        gathered_info[full_name] = func
    else:
        assert isinstance(context_stack[-1], ClassInfo)
        context_stack[-1].methods.append(func)

    context_stack.append(func)


def parse_overload(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_OVERLOAD macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    name = parse_string(args[0])
    ret = parse_string(args[1])

    assert isinstance(context_stack[-1], FuncInfo), "overload expected previous function"
    assert context_stack[-1].name == name, "overload must have same name as previous function"

    if context_stack[-1].overloads is None:
        # The last entry on the stack *is* an overload, go back to the main function
        context_stack.pop()

        assert isinstance(context_stack[-1], FuncInfo), "overload expected previous function"
        assert context_stack[-1].name == name, "overload must have same name as previous function"
        assert context_stack[-1].overloads is not None, "found nested overload"

    func = _create_func(context_stack[-1].func_type, name, ret)
    func.overloads = None

    context_stack[-1].overloads.append(func)
    context_stack.append(func)


def parse_arg(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_ARG macro.

    Args:
        args: The macro's args.
    """

    assert len(args) == 3, "expected 3 args"  # noqa: PLR2004

    assert args[0][-1].type == "CPP_ID" and args[0][-1].value == "_a", (
        'expected first arg to be a pybind "arg"_a literal'
    )
    name = parse_string(args[0][:-1])

    type_hint = parse_string(args[1])
    default = parse_string(args[2]) if args[2] else None

    assert isinstance(context_stack[-1], FuncInfo), "tried to add an arg outside of a function"
    context_stack[-1].args.append(ArgInfo(name, type_hint, default))


def parse_pos_only_kw_only(macro_name: str, args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_POS_ONLY or PYUNREALSDK_STUBGEN_KW_ONLY macro.

    Args:
        macro_name: The name of the macro being parsed.
        args: The macro's args.
    """
    assert len(args) == 1 and not args[0], "expected no args"
    assert isinstance(context_stack[-1], FuncInfo), (
        "tried to add an arg marker outside of a function"
    )
    context_stack[-1].args.append(
        ArgInfo(
            "/" if macro_name == "PYUNREALSDK_STUBGEN_POS_ONLY" else "*",
            None,
            None,
        ),
    )


def parse_enum(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_ENUM macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 1, "expected one arg"
    name = parse_string(args[0])

    while not isinstance(context_stack[-1], ModuleInfo):
        context_stack.pop()

    full_name = ".".join(x.name for x in context_stack) + "." + name
    assert full_name not in gathered_info, f"got duplicate enum {full_name}"

    enum = EnumInfo(name)
    gathered_info[full_name] = enum
    context_stack.append(enum)


def parse_class(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_CLASS macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    name = parse_string(args[0])
    super_class = parse_string(args[1]) if args[1] else None

    while not isinstance(context_stack[-1], ModuleInfo):
        context_stack.pop()

    full_name = ".".join(x.name for x in context_stack) + "." + name
    assert full_name not in gathered_info, f"got duplicate class {full_name}"

    cls = ClassInfo(name, super_class)
    gathered_info[full_name] = cls
    context_stack.append(cls)


def parse_deprecated_generic(macro_name: str, args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_DEPRECATED or PYUNREALSDK_STUBGEN_GENERIC macro.

    Args:
        macro_name: The name of the macro being parsed.
        args: The macro's args.
    """
    assert len(args) == 1, "expected one arg"
    value = parse_string(args[0])

    error_type = "deprecated" if macro_name == "PYUNREALSDK_STUBGEN_DEPRECATED" else "generic"
    assert isinstance(context_stack[-1], ClassInfo | FuncInfo), (
        f"can only add {error_type} to class, function or method"
    )

    if macro_name == "PYUNREALSDK_STUBGEN_DEPRECATED":
        assert context_stack[-1].deprecated is None, (
            "tried to add deprecation message to object which already has one"
        )
        context_stack[-1].deprecated = value
    else:
        assert context_stack[-1].generic is None, (
            "tried to add generic to object which already has one"
        )
        context_stack[-1].generic = value


def parse_file(path: Path, flavour: Flavour) -> InfoDict:  # noqa: C901
    """
    Parses all data out of the given file.

    Args:
        path: The file to parse.
        flavour: What SDK flavour to parse using.
    Returns:
        The parsed info.
    """
    gathered_info.clear()
    context_stack.clear()

    for macro, args in parse_macros_from_file(path, flavour):
        try:
            macro_name = macro.name.removesuffix("_N")
            match macro_name:
                case "PYUNREALSDK_STUBGEN_MODULE":
                    parse_module(args)
                case "PYUNREALSDK_STUBGEN_SUBMODULE":
                    parse_submodule(args)
                case "PYUNREALSDK_STUBGEN_DOCSTRING":
                    parse_docstring(args)
                case "PYUNREALSDK_STUBGEN_ATTR":
                    while isinstance(context_stack[-1], FuncInfo):
                        context_stack.pop()

                    match context_stack[-1]:
                        case EnumInfo() | EnumValueInfo():
                            parse_enum_attr(args)
                        case ClassInfo():
                            parse_class_attr(args)
                        case _:
                            parse_module_attr(args)
                case "PYUNREALSDK_STUBGEN_READONLY_PROP":
                    parse_readonly_prop(args)
                case "PYUNREALSDK_STUBGEN_FUNC":
                    parse_func(args, FuncType.Func)
                case "PYUNREALSDK_STUBGEN_METHOD":
                    parse_func(args, FuncType.Method)
                case "PYUNREALSDK_STUBGEN_STATICMETHOD":
                    parse_func(args, FuncType.StaticMethod)
                case "PYUNREALSDK_STUBGEN_CLASSMETHOD":
                    parse_func(args, FuncType.ClassMethod)
                case "PYUNREALSDK_STUBGEN_OVERLOAD":
                    parse_overload(args)
                case "PYUNREALSDK_STUBGEN_ARG":
                    parse_arg(args)
                case "PYUNREALSDK_STUBGEN_POS_ONLY" | "PYUNREALSDK_STUBGEN_KW_ONLY":
                    parse_pos_only_kw_only(macro_name, args)
                case "PYUNREALSDK_STUBGEN_ENUM":
                    parse_enum(args)
                case "PYUNREALSDK_STUBGEN_CLASS":
                    parse_class(args)
                case "PYUNREALSDK_STUBGEN_DEPRECATED" | "PYUNREALSDK_STUBGEN_GENERIC":
                    parse_deprecated_generic(macro_name, args)
                case "PYUNREALSDK_STUBGEN_NEVER_METHOD":
                    # Ignore, want to parse what this expands to instead
                    pass
                case _:
                    assert not macro.name.startswith("PYUNREALSDK_STUBGEN"), (
                        "encountered unknown stubgen macro"
                    )
        except Exception as ex:
            try:
                path_str = str(path.relative_to(Path(__file__).parent.parent))
            except ValueError:
                path_str = str(path)
            sys.stderr.write(f"Failed to parse macro from {path_str}:\n{macro} {args}\n")
            raise ex

    return gathered_info
