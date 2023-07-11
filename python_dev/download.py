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

def download_python_msis(version: str, arch: str, download_dir: Path) -> tuple[str, str]:
    msi_url_base = f"https://www.python.org/ftp/python/{version}/{arch}/"

    paths = []
    for file_name in ("dev.msi", "dev_d.msi"):
        path = download_dir / file_name
        paths.append(path)

        download_file(msi_url_base + file_name, path)

    return tuple(paths)

def extract_msi(msi: Path, extract_dir: Path) -> None:
    if platform.system() == "Windows":
        # msiexec doesn't like extracting to the same dir the file is in, so extract to a temp
        # nested dir first
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
        ], check=True)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("version", type=str, help="The version to download.")
    parser.add_argument("arch", choices=("win32", "amd64"), help="The architecture to download.")

    base_dir = find_dev_files_dir()
    parser.add_argument(
        "--dir",
        default=base_dir, required=base_dir is None,
        help="The base directory to download into."
    )

    args = parser.parse_args()

    download_dir = args.dir / ARCH_DIR_MAPPING[args.arch]
    download_dir.mkdir(exist_ok=True)

    dev, dev_d = download_python_msis(args.version, args.arch, download_dir)

    extract_msi(dev, download_dir)
    extract_msi(dev_d, download_dir)
