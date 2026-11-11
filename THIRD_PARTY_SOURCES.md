# Runtime source access

## GCC runtime libraries

Bundled libstdc++6 and libgcc-s1 were taken without code changes from Debian 12 **12.2.0-14+deb12u1 (amd64)**. Their complete source package is available from the Debian archive. These source downloads accompany the object-code release through the following equivalent network access:

- [Signed source-package description](https://deb.debian.org/debian/pool/main/g/gcc-12/gcc-12_12.2.0-14+deb12u1.dsc)
- [Original GCC source](https://deb.debian.org/debian/pool/main/g/gcc-12/gcc-12_12.2.0.orig.tar.gz) — SHA-256 b8298be16aeeb96a889c6afed0a8e2241b47452e89cc81fe65ea849d5c740fcb
- [Debian patches and build rules](https://deb.debian.org/debian/pool/main/g/gcc-12/gcc-12_12.2.0-14+deb12u1.debian.tar.xz) — SHA-256 59f7f7763a0c355e3f27ff9e7ac80d06382b29939361a87e7b139226bfe7402e

The signed .dsc is also included in Source/ThirdParty/. Download both the original source and Debian archive for the corresponding source package. The Debian archive includes the packaging/build rules. License and runtime-exception texts are included in licenses/GCC-runtime-copyright.txt and licenses/GPL-3.txt.

## MIT runtime projects

- [Box64 upstream](https://github.com/ptitSeb/box64), recorded base commit 1a49e470758276d49b2d09a78e199e12951c6dc3.
- [GL4ES upstream](https://github.com/ptitSeb/gl4es), recorded base commit 2037865590d410bcd7b8d1f22b841472ed6d39ac.
- [PolluxPT Braid RGDS+ project](https://github.com/PolluxPT/Braid-RGDSPlus), source of the tested modified runtime binaries. The runtime files are not claimed to be byte-for-byte builds of the unmodified upstream commits. Their MIT notices are included.

New Shovel Knight integration source is included directly under Source/. No vendor game or FMOD headers are distributed.
