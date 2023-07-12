import argparse
import platform
import shutil
import subprocess
import tempfile
import zipfile
from pathlib import Path

import requests

DEV_FILES_DIRNAME: str = "python_dev"

ARCH_DIR_MAPPING: dict[str, str] = {
    "win32": "x86",
    "amd64": "x64",
}

def find_dev_files_dir() -> Path | None:
    cwd = Path.cwd()
    if cwd.name == DEV_FILES_DIRNAME:
        return cwd
    for subdir in cwd.iterdir():
        if subdir.name == DEV_FILES_DIRNAME:
            return subdir

    return None

def download_file(url: str, path: Path) -> None:
    with requests.get(url, stream=True) as resp, open(path, "wb") as file:
        resp.raw.decode_content = True
        shutil.copyfileobj(resp.raw, file)

def extract_msi(msi: Path, extract_dir: Path) -> None:
    if platform.system() == "Windows":
        # msiexec doesn't like extracting to the same dir the file is in, so extract to a temp dir
        with tempfile.TemporaryDirectory() as tmp_dir:
            ret = subprocess.run([
                "msiexec",
                f"TARGETDIR={tmp_dir}",
                "/a", str(msi),
                "/qn"
            ])
            if ret.returncode == 2203:
                print("Do you have permission to write to the output dir?")
            ret.check_returncode()

            # When using /a, msiexec also adds an extracted msi which we don't want
            for extracted_msi in Path(tmp_dir).rglob("*.msi"):
                extracted_msi.unlink()

            shutil.copytree(tmp_dir, extract_dir, dirs_exist_ok=True)
    else:
        subprocess.run([
            "msiextract",
            "-C", str(extract_dir),
            str(msi)
        ], check=True, stdout=subprocess.DEVNULL)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("version", type=str, help="The version to download.")
    parser.add_argument("arch", choices=("win32", "amd64"), help="The architecture to download.")
    parser.add_argument(
        "--debug",
        action=argparse.BooleanOptionalAction, default=True,
        help="Include the debug files. Defaults on."
    )
    parser.add_argument(
        "--stdlib",
        action=argparse.BooleanOptionalAction, default=True,
        help="Include the standard library (including dlls + zip). Defaults on."
    )

    base_dir = find_dev_files_dir()
    parser.add_argument(
        "--dir",
        default=base_dir, required=base_dir is None,
        help="The base directory to download into."
    )

    args = parser.parse_args()

    download_dir = args.dir / ARCH_DIR_MAPPING[args.arch]
    download_dir.mkdir(exist_ok=True)

    msi_url_base = f"https://www.python.org/ftp/python/{args.version}/{args.arch}/"

    # msis we just extract straight to the download folder
    basic_msis = [
        "core.msi",
        "dev.msi"
    ]
    if args.stdlib:
        basic_msis.append("lib.msi")

    if args.debug:
        # Include the _d versions
        basic_msis += [msi.removesuffix(".msi") + "_d.msi" for msi in basic_msis]

    for msi_name in basic_msis:
        download_path = download_dir / msi_name

        download_file(msi_url_base + msi_name, download_path)
        extract_msi(download_path, download_dir)

        download_path.unlink()

    if args.stdlib:
        # Download the embedded file to get the precompiled zip, rather than compile it ourselves,
        # since the python we're running in may not be the same as what we're downloading
        embed_url_base = f"https://www.python.org/ftp/python/{args.version}/"
        embed_name = f"python-{args.version}-embed-{args.arch}.zip"

        embed_download_path = download_dir / embed_name
        download_file(embed_url_base + embed_name, embed_download_path)

        with zipfile.ZipFile(embed_download_path, "r") as file:
            for inner in file.infolist():
                if not inner.filename.endswith(".zip"):
                    continue
                file.extract(inner, download_dir)

                if args.debug:
                    # Debug mode needs a different filename, but is otherwise identical
                    extracted_file = download_dir / inner.filename
                    debug_copy = download_dir / (extracted_file.stem + "_d.zip")
                    shutil.copy(extracted_file, debug_copy)

        embed_download_path.unlink()
