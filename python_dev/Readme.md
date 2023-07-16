This folder should contain (symlinks to) the python headers and static libraries. These should be
placed in `x86/include` and `x86/libs`, or under `x64/` for 64 bit versions.

On Windows, this is equivalent to symlinking these folders to the python install folder. For
convenience, the script `symlink_this.py` does just that, with the python install it's run with.
Make sure you have permission to create symlinks when running it - either enable Windows dev mode or
run it as admin.

You can also download and extract these fails straight out of the python installers.
```sh
URL=https://www.python.org/ftp/python/3.11.4/amd64
wget $URL/dev.msi $URL/dev_d.msi

# On Windows
msiexec TARGETDIR={tmp_dir} /a dev.msi /qn
msiexec TARGETDIR={tmp_dir} /a dev_d.msi /qn
# On Linux/Mac
msiextract -C x64 dev.msi dev_d.msi
```

The script `download.py <version> <arch>` will do just this. It requires the python `requests`
module. If not on Windows, it also requires `msiextract` (from `msitools`) to be on your path.

When installing this project with CMake, it additionally looks for files matching:
```
(x86|x64)/DLLs/*.dll
(x86|x64)/DLLs/*.pyd
(x86|x64)/DLLs/*.zip
(x86|x64)/*.dll
(x86|x64)/*.pyd
(x86|x64)/*.zip
```
These files will also be copied to the install dir if they exist, since you need them to get python
running. It is not an error if they don't exist, you can compile fine without them, they're just
needed at runtime.

If you use `download.py`, they will all automatically be downloaded.

If you have a standard Windows Python install, including all the development libraries, and used
`symlink_this.py`, you will have everything *except for* the zips holding the standard library. You
will instead have them extracted in the `Lib` folder. There are three ways to get a zip:
- Just zip the `Lib` up folder, and rename it to `python3<version>.zip` (or `python3<version>_d.zip`
  for debug)
- Download the python embeddable package for the same version, and copy the zip from there. This
  version contains precompiled bytecode instead of raw source files.
- Create your own compiled zip using `zipfile.PyZipFile`.
