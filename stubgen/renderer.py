import subprocess
from collections.abc import Sequence
from pathlib import Path
from pprint import pprint
from typing import Any

# Being slightly overzealous with the pyright ignores so that this will work even if you don't have
# jinja installed in the same environment as pyright
from jinja2 import (  # pyright: ignore[reportMissingImports]
    Environment,  # pyright: ignore[reportUnknownVariableType]
    FileSystemLoader,  # pyright: ignore[reportUnknownVariableType]
    select_autoescape,  # pyright: ignore[reportUnknownVariableType]
)

from .info import InfoDict, ModuleInfo

__all__: tuple[str, ...] = ("render_stubs",)


def render_stubs(
    output_dir: Path,
    info: InfoDict,
    flavour_macros: dict[str, Any] | None = None,
) -> None:
    """
    Renders all stub files.

    Args:
        output_dir: The dir to write the stubs into.
        info: The collected stub info.
        flavour_macros: Sdk flavour macros to inject into the global jinja namespace.
    """
    loader = FileSystemLoader(Path(__file__).parent / "templates")  # pyright: ignore[reportUnknownVariableType]
    env = Environment(loader=loader, autoescape=select_autoescape())  # pyright: ignore[reportUnknownVariableType]

    all_names = set(info.keys())

    def _all_(value: str) -> str:
        assert isinstance(value, str), "expected a string arg"
        return (
            "__all__: tuple[str, ...] = (\n"
            + "".join(
                f'    "{partition[2]}",\n'
                for name in all_names
                if (partition := name.rpartition("."))[0] == value
            )
            + ")\n"
        )

    def declare(value: str) -> str:
        assert isinstance(value, str), "expected a string arg"
        return info.pop(value).declare()

    def declare_all(values: Sequence[str]) -> str:
        assert isinstance(values, Sequence) and isinstance(values[0], str), (
            "expected a sequence of strings"
        )
        return "\n".join(declare(val) for val in sorted(values))

    env.filters["__all__"] = _all_  # pyright: ignore[reportUnknownMemberType]
    env.filters["declare"] = declare  # pyright: ignore[reportUnknownMemberType]
    env.filters["declare_all"] = declare_all  # pyright: ignore[reportUnknownMemberType]

    env.globals |= flavour_macros or {}  # pyright: ignore[reportUnknownMemberType]

    for name in loader.list_templates():  # pyright: ignore[reportUnknownMemberType, reportUnknownVariableType]
        template = env.get_template(name)  # pyright: ignore[reportUnknownMemberType, reportUnknownVariableType]
        output = (output_dir / name).with_suffix("")  # pyright: ignore[reportUnknownMemberType, reportUnknownVariableType]
        output.parent.mkdir(parents=True, exist_ok=True)  # pyright: ignore[reportUnknownMemberType]
        output.write_text(template.render())  # pyright: ignore[reportUnknownMemberType]

    # Allow not rendering modules if they don't have a docstring
    remaining = {
        k: v for k, v in info.items() if not (isinstance(v, ModuleInfo) and v.docstring is None)
    }
    if remaining:
        pprint(remaining, width=180)  # noqa: T203
        assert not remaining, "above entries weren't consumed by templates"

    subprocess.run(["ruff", "format", output_dir], check=True)
    subprocess.run(["ruff", "check", "--fix", output_dir], check=True)
    subprocess.run(["ruff", "format", output_dir], check=True)
