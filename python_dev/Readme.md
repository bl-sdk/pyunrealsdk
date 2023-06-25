This folder should contain (symlinks to) the python headers and static libraries. These should be
placed in `x86/include` and `x86/libs`, or under `x64/` for 64 bit versions. On Windows, this is
equivalent to symlinking these folders to the python install folder.

For convenience, the script `symlink_this.py` will create the relevant symlinks for the python
install it's run with. Make sure you have permission to create symlinks when running it - either
enable Windows dev mode or run it as admin.

When cross compiling, you can extract the Windows dev files using `msitools`:
```sh
URL=https://www.python.org/ftp/python/3.11.4/amd64
wget $URL/dev.msi $URL/dev_d.msi
msiextract -C x64 dev.msi dev_d.msi
```
