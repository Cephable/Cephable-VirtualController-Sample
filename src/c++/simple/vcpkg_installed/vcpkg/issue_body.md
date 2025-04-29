Package: openssl:arm64-osx@3.5.0#1

**Host Environment**

- Host: arm64-osx
- Compiler: AppleClang 16.0.0.16000026
-    vcpkg-tool version: 2025-04-16-f9b6c6917b23c1ccf16c1a9f015ebabf8f615045
    vcpkg-scripts version: b66385232e 2025-04-28 (4 hours ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
Downloading https://github.com/openssl/openssl/commit/2b5e7253b9a6a4cde64d3f2f22d71272f6ad32c5.patch?full_index=1 -> openssl-certstore-crash-2b5e7253b9a6a4cde64d3f2f22d71272f6ad32c5.patch
Successfully downloaded openssl-certstore-crash-2b5e7253b9a6a4cde64d3f2f22d71272f6ad32c5.patch
Downloading https://github.com/openssl/openssl/archive/openssl-3.5.0.tar.gz -> openssl-openssl-openssl-3.5.0.tar.gz
Successfully downloaded openssl-openssl-openssl-3.5.0.tar.gz
-- Extracting source /Users/alexdunn/Projects/Cephable-VirtualController-Sample/src/c++/simple/vcpkg/downloads/openssl-openssl-openssl-3.5.0.tar.gz
-- Applying patch cmake-config.patch
-- Applying patch command-line-length.patch
-- Applying patch script-prefix.patch
-- Applying patch windows/install-layout.patch
-- Applying patch windows/install-pdbs.patch
-- Applying patch /Users/alexdunn/Projects/Cephable-VirtualController-Sample/src/c++/simple/vcpkg/downloads/openssl-certstore-crash-2b5e7253b9a6a4cde64d3f2f22d71272f6ad32c5.patch
-- Applying patch unix/android-cc.patch
-- Applying patch unix/move-openssldir.patch
-- Applying patch unix/no-empty-dirs.patch
-- Applying patch unix/no-static-libs-for-shared.patch
-- Using source at /Users/alexdunn/Projects/Cephable-VirtualController-Sample/src/c++/simple/vcpkg/buildtrees/openssl/src/nssl-3.5.0-3b91ebed43.clean
-- Getting CMake variables for arm64-osx
-- Getting CMake variables for arm64-osx-dbg
-- Getting CMake variables for arm64-osx-rel
CMake Error at scripts/cmake/vcpkg_find_acquire_program.cmake:166 (message):
  Could not find pkg-config.  Please install it via your package manager:

      brew install pkg-config
Call Stack (most recent call first):
  scripts/cmake/z_vcpkg_setup_pkgconfig_path.cmake:19 (vcpkg_find_acquire_program)
  scripts/cmake/vcpkg_configure_make.cmake:816 (z_vcpkg_setup_pkgconfig_path)
  ports/openssl/unix/portfile.cmake:111 (vcpkg_configure_make)
  ports/openssl/portfile.cmake:80 (include)
  scripts/ports.cmake:206 (include)



```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "dependencies": [
    "cpprestsdk",
    "uwebsockets"
  ]
}

```
</details>
