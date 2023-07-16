# SDK Stub Files
You can point your ide at these files to get type checking and docstrings for sdk functions.

## Generating Draft Stubs
`mypy`'s `stubgen` and `pybind11-stubgen` provide tools to automatically generate draft stubs.
However, they're made to be run on `.pyd`s, and not against an embedded module. With a bit of
coercing we can get them to work however.

```py
import sys

sys.path.append("path/to/site-packages")

import pybind11_stubgen
from mypy import stubgen

# Force all modules to be treated as c modules
stubgen.find_module_path_and_all_py3 = lambda *_: None

stubgen.generate_stubs(stubgen.Options(
    pyversion=sys.version_info[:2],
    no_import=False,
    doc_dir="",
    search_path="",
    interpreter=None,
    ignore_errors=False,
    parse_only=False,
    include_private=True,
    output_dir="",
    modules=[
        "pyunrealsdk",
        "unrealsdk",
        "unrealsdk.commands",
        "unrealsdk.hooks",
        "unrealsdk.logging",
        "unrealsdk.unreal",
    ],
    packages=[],
    files=[],
    verbose=True,
    quiet=False,
    export_less=False,
))

module = pybind11_stubgen.ModuleStubsGenerator("unrealsdk")
module.parse()
module.write()
```

The output of both generators was manually combined to create these stubs.
