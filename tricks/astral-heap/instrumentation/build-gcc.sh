#!/usr/bin/env bash
# Build only the GCC front ends needed for header-free constexpr observations.
set -euo pipefail
trace_here=$(cd -- "$(dirname -- "$0")" && pwd)
trace_work=${1:?Usage: build-gcc.sh /absolute/path/to/build-work}
mkdir -p "$trace_work"
trace_work=$(cd "$trace_work" && pwd)
cd "$trace_work"
if [[ ! -d gcc-13.3.0 ]]; then
    curl --fail --location --retry 2 https://ftp.gnu.org/gnu/gcc/gcc-13.3.0/gcc-13.3.0.tar.xz -o gcc-13.3.0.tar.xz
    tar --no-same-owner -xf gcc-13.3.0.tar.xz
fi
cd gcc-13.3.0
for trace_archive in gmp-6.2.1.tar.bz2 mpfr-4.1.0.tar.bz2 mpc-1.2.1.tar.gz; do
    if [[ ! -f "$trace_archive" ]]; then
        curl --fail --location --retry 2 "https://gcc.gnu.org/pub/gcc/infrastructure/$trace_archive" -o "$trace_archive"
    fi
done
# The release helper verifies these archives against its shipped SHA512 list.
TAR_OPTIONS="${TAR_OPTIONS:-} --no-same-owner" ./contrib/download_prerequisites --no-isl
if ! rg -q 'GCC_ASTRAL_TRACE' gcc/cp/constexpr.cc; then
    patch -p1 < "$trace_here/gcc-13.3-astral-trace.patch"
fi
mkdir -p "$trace_work/build"
cd "$trace_work/build"
if [[ ! -f Makefile ]]; then
    ../gcc-13.3.0/configure --disable-bootstrap --disable-multilib --disable-nls \
        --disable-libsanitizer --disable-libquadmath --disable-libgomp \
        --disable-libatomic --disable-libssp --disable-libvtv --without-isl \
        --without-zstd --enable-languages=c,c++ --enable-checking=release \
        CFLAGS='-O0 -g0' CXXFLAGS='-O0 -g0' > configure.log 2>&1
fi
make -j"${TRACE_JOBS:-8}" all-gcc > build.log 2>&1
echo "Compiler: $trace_work/build/gcc/cc1plus"
