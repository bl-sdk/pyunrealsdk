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
