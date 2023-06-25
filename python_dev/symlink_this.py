import platform
import sys
import traceback
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator

DEV_FILES_DIRNAME: str = "python_dev"

def find_dev_files_dir() -> Path:
    cwd = Path.cwd()
    if cwd.name == DEV_FILES_DIRNAME:
        return cwd
    for subdir in cwd.iterdir():
        if subdir.name == DEV_FILES_DIRNAME:
            return subdir

    raise RuntimeError(
        f"Unable to find the '{DEV_FILES_DIRNAME}' directory to make symlinks in. Aborting."
    )

def get_arch_dirname() -> bool:
    # Recommended over `platform.architecture()[0] == "64bit"`
    is_64_bit = sys.maxsize > 2**32
    return "x64" if is_64_bit else "x86"

def in_terminal_session() -> bool:
    if platform.system() == "Windows":
        # https://stackoverflow.com/a/73824277
        try:
            with open('CONIN$'):
                return True
        except:
            return False
    else:
        return sys.stdin.isatty()

@contextmanager
def enter_to_exit(force: bool = False) -> Iterator[None]:
    should_show = force or not in_terminal_session()

    try:
        yield
    except Exception as ex:
        if should_show:
            traceback.print_exc()
        raise ex
    finally:
        if should_show:
            input("\nPress enter to exit")

if __name__ == "__main__":
    with enter_to_exit():
        if platform.system() != "Windows":
            raise RuntimeError(
                "This script only works on Windows, due to assumptions about the install layout."
            )

        # This is what won't work cross platform
        # On Windows, the include and lib dirs both just exist in the same dir as the executable

        # It's trivial enough to find the include dir, `sysconfig.get_path("include")`
        # The static libs are more difficult - and given we're only building against windows for now
        #  anyway, not going to put in the effort to work it out yet
        py_install_dir = Path(sys.executable).parent

        dest = (find_dev_files_dir() / get_arch_dirname())
        if dest.exists():
            dest.rmdir()
        dest.symlink_to(py_install_dir, True)
