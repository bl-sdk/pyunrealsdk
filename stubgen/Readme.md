# Stub File Generators
The scripts in this folder help generate a set of type stubs for a given sdk flavour. There are a
few subtle differences between flavours, this takes them into account, and keeps the stubs up to
date with any changes in the source code automatically.

They are intended to be run as a module, i.e. from the project root:
```sh
pip install -r stubgen/requirements.txt
python -m stubgen WILLOW
```

Alternatively, if you set up your python path appropriately, you can run it programmatically.
```py
import stubgen
stubgen.generate(stubgen.Flavour.OAK, Path("output"))
```

### How it works
Firstly, we have to gather all the definitions from the C++ source code.

Rather than trying to parse arbitrary C++, we define a number of macros in `pyunrealsdk/stubgen.h`,
and simply look for them. These macros are parsed *after* applying any ifdef using variables from
`unrealsdk/flavour.h`, so that any behaviour switched on them also change what Python metadata we
pull out.

Most of these macros evaluate directly to one of their args, acting as passthroughs, while also
taking extra args to note down Python-specific metadata. For example, `PYUNREALSDK_STUBGEN_ATTR`
takes a name and a type hint, but only expands to the name. This allows it to be used inside a
pybind `.attr()` expression, directly where we're defining the attribute.

```cpp
mod.attr(PYUNREALSDK_STUBGEN_ATTR("my_var", "int")) = 3;
// becomes:
mod.attr("my_var") = 3;
```

All macros also have a `_N` null macro variant, which expands to nothing, which can be used when
there isn't anything valid in C++ to expand to.

In order to facilitate these kinds of simple passthroughs with inline metadata, the parsing uses a
context stack. Most macros attach to the last macro of a given type - which sometimes also pops
entries of another type. For example, `PYUNREALSDK_STUBGEN_ARG` attaches to the last seen
`PYUNREALSDK_STUBGEN_FUNC`, and `PYUNREALSDK_STUBGEN_FUNC` itself will pop any other functions on
the stack until it finds the last class or module. The exact semantics of this are very ad-hoc,
these scripts are not meant to be general purpose, as long as they can parse our source code they're
good enough.

Each `.cpp` file is parsed separately (with it's own context stack), strictly linearly from top to
bottom. If something is defined inside a loop in C++, or by calling some function repeatedly with
different args, these scripts will not be able to pick them up. Instead, we need extra macros to
declare each variants. This *can* use higher level macros.

```cpp
void declare_adder(py::module_& mod, const char* name, int value) {
    mod.def(name, [value](int base) { return base + value; }, "adds a number", "base"_a);
}

#define STUBGEN_ADDER(name)                          \
    PYUNREALSDK_STUBGEN_FUNC_N(name, "int")          \
    PYUNREALSDK_STUBGEN_DOCSTRING_N("adds a number") \
    PYUNREALSDK_STUBGEN_ARG_N("base"_a, "int",)

// ...

declare_adder(mod, "add_three", 3);
declare_adder(mod, "add_five", 5);

STUBGEN_ADDER("add_three")
STUBGEN_ADDER("add_five")
```

After pulling all info out of the source code, we have to inject it into the stub files. This uses
all the files in the templates folder. These use jinja templates.

The philosophy we take is, rather than try generate absolutely everything automatically, we only use
these scripts to pull in all the declarations from the source code, and still mostly hand-write the
stubs. The templates files declare the general structure of each file - e.g. all the imports, custom
type hints, the exact order of each class - and the only magic we use is to pull in the
declarations. If adding something non-trivial, it may be easier to first write it in the generated
stubs folder, then copy it back into the templates after.

The following custom filters are available in templates:
- `{{ "unrealsdk" | __all__ }}` writes the `__all__` list for the given module.
- `{{ "unrealsdk.find_object" | declare }}` declares the given object.
- Given an array of object names:
  ```jinja
  {% set classes = [
      "unrealsdk.unreal.UConst",
      "unrealsdk.unreal.UStruct",
      "unrealsdk.unreal.ZProperty",
  ] %}
  {{ classes | declare_all }}
  ```
  Sorts all items in the array, then declares them one by one.

Additionally, all flavour defines from `unrealsdk/flavour.h` are available as global variables.
```jinja
{% if UNREALSDK_PROPERTIES_ARE_FFIELD %}
from ._uobject_children import ZProperty
{% endif %}
```
