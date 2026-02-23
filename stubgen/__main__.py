from pathlib import Path

from . import generate
from .preprocessor import Flavour

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "flavour",
        type=Flavour,
        choices=Flavour,
        help="Which SDK flavour to build stubs for",
    )
    parser.add_argument(
        "output",
        nargs="?",
        default=Path(__file__).parent / ".out",
        type=Path,
        help="Where to write the stubs to.",
    )
    args = parser.parse_args()

    generate(args.flavour, args.output)
