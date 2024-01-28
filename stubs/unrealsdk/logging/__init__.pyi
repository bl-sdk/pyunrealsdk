# ruff: noqa: D205

from __future__ import annotations

from typing import Any, ClassVar

__all__: tuple[str, ...] = (
    "Level",
    "Logger",
    "dev_warning",
    "error",
    "info",
    "is_console_ready",
    "misc",
    "set_console_level",
    "warning",
)

class Level:
    """
    Enum of valid logging levels.

    Members:

      ERROR : Used to display error messages.

      WARNING : Used to display warnings.

      INFO : Default logging level, used for anything that should be shown in console.

      DEV_WARNING : Used for warnings which don't concern users, so shouldn't be shown in console.

      MISC : Used for miscellaneous debug messages.
    """

    __members__: ClassVar[dict[str, Level]]
    __entries: ClassVar[dict[str, tuple[Level, str]]]

    DEV_WARNING: ClassVar[Level]
    ERROR: ClassVar[Level]
    INFO: ClassVar[Level]
    MISC: ClassVar[Level]
    WARNING: ClassVar[Level]

    def __eq__(self, other: object) -> bool: ...
    def __getstate__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __init__(self, value: int) -> None: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: object) -> bool: ...
    def __repr__(self) -> str: ...
    def __setstate__(self, state: int) -> None: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class Logger:
    """A write only file object which redirects to the unrealsdk log system."""

    level: Level

    def __init__(self, level: Level = Level.INFO) -> None:
        """
        Creates a new logger.

        Args:
            level: The default log level to initialize to.
        """
    def flush(self) -> None:
        """Flushes the stream."""
    def write(self, text: str) -> int:
        """
        Writes a string to the stream.

        Args:
            text: The text to write.
        Returns:
            The number of chars which were written (which is always equal to the length
            of the string).
        """

def dev_warning(*args: Any, **kwargs: Any) -> None:
    """
    Wrapper around print(), which temporarily changes the log level of stdout to
    dev warning.

    Args:
        *args: Forwarded to print().
        **kwargs: Forwarded to print().
    """

def error(*args: Any, **kwargs: Any) -> None:
    """
    Wrapper around print(), which temporarily changes the log level of stdout to
    error.

    Args:
        *args: Forwarded to print().
        **kwargs: Forwarded to print().
    """

def info(*args: Any, **kwargs: Any) -> None:
    """
    Wrapper around print(), which temporarily changes the log level of stdout to
    info.

    Args:
        *args: Forwarded to print().
        **kwargs: Forwarded to print().
    """

def is_console_ready() -> bool:
    """
    Checks if the sdk's console hook is ready to output text.

    Anything written before this point will only be visible in the log file.

    Returns:
        True if the console hook is ready, false otherwise.
    """

def misc(*args: Any, **kwargs: Any) -> None:
    """
    Wrapper around print(), which temporarily changes the log level of stdout to
    misc.

    Args:
        *args: Forwarded to print().
        **kwargs: Forwarded to print().
    """

def set_console_level(level: Level) -> bool:
    """
    Sets the log level of the unreal console.

    Does not affect the log file or external console, if enabled.

    Args:
        level: The new log level.
    Returns:
        True if console level changed, false if an invalid value was passed in.
    """

def warning(*args: Any, **kwargs: Any) -> None:
    """
    Wrapper around print(), which temporarily changes the log level of stdout to
    warning.

    Args:
        *args: Forwarded to print().
        **kwargs: Forwarded to print().
    """
