#!/usr/bin/env bash
# mayhem/build.sh — build GMT twice and the libFuzzer module harnesses.
#   1) mayhem/build-fuzz -> mayhem/out/fuzz : libgmt, libpostscriptlight and the supplements plugin
#      with $SANITIZER_FLAGS + SanCov (-fsanitize=fuzzer-no-link) + DWARF 3.
#   2) mayhem/build-test -> mayhem/out/test : the same tree with NORMAL flags; mayhem/test.sh runs its
#      `gmt` CLI as the functional oracle.
#   3) mayhem/out/bin/fuzz_<name> (+ -standalone) from mayhem/harnesses/.
# Re-runnable (CMake/Ninja are incremental) and offline (all deps come from apt in the image).
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
: "${SRC:=/mayhem}"
export CC CXX
cd "$SRC"

M="$SRC/mayhem"
FUZZ_PREFIX="$M/out/fuzz"
TEST_PREFIX="$M/out/test"
BIN="$M/out/bin"
mkdir -p "$BIN"

COMMON_CMAKE=(-GNinja -DCMAKE_BUILD_TYPE=Release -DGMT_ENABLE_OPENMP=OFF -DGMT_USE_THREADS=OFF
              -DGMT_INSTALL_MODULE_LINKS=OFF -DCMAKE_SKIP_INSTALL_RPATH=OFF)

# 1) Sanitized, coverage-instrumented library build (what the harnesses fuzz).
# Two UBSan checks are relaxed (ASan and every other UBSan check stay on and halting):
#  - nonnull-attribute: every session calls bsearch(key, NULL, 0, ...) in
#    gmt_remote.c gmt_remote_dataset_id when no remote-server index exists (always, offline), which
#    would abort every input before any parsing.
#  - pointer-overflow: GMT forms null+0 and p+(unsigned)-1 pointers all over its readers (e.g.
#    spotter.c on an empty rotation file, gmt_io.c:2930, pssegy.c:690); they flood every target
#    within seconds without an out-of-bounds access, which ASan still catches.
FUZZ_CFLAGS="-O1 $SANITIZER_FLAGS -fno-sanitize=nonnull-attribute,pointer-overflow -fsanitize=fuzzer-no-link $DEBUG_FLAGS"
cmake -S "$SRC" -B "$M/build-fuzz" "${COMMON_CMAKE[@]}" \
  -DCMAKE_INSTALL_PREFIX="$FUZZ_PREFIX" \
  -DCMAKE_C_FLAGS="$FUZZ_CFLAGS" -DCMAKE_C_FLAGS_RELEASE="" \
  -DCMAKE_SHARED_LINKER_FLAGS="$SANITIZER_FLAGS" \
  -DCMAKE_MODULE_LINKER_FLAGS="$SANITIZER_FLAGS" \
  -DCMAKE_EXE_LINKER_FLAGS="$SANITIZER_FLAGS -Wl,--allow-shlib-undefined" \
  -DCMAKE_INSTALL_RPATH="$FUZZ_PREFIX/lib"
cmake --build "$M/build-fuzz" -j"$MAYHEM_JOBS"
cmake --install "$M/build-fuzz" >/dev/null

# 2) Normal-flags build for the functional oracle.
cmake -S "$SRC" -B "$M/build-test" "${COMMON_CMAKE[@]}" \
  -DCMAKE_INSTALL_PREFIX="$TEST_PREFIX" \
  -DCMAKE_C_FLAGS="$COVERAGE_FLAGS" -DCMAKE_EXE_LINKER_FLAGS="$COVERAGE_FLAGS" \
  -DCMAKE_SHARED_LINKER_FLAGS="$COVERAGE_FLAGS" -DCMAKE_MODULE_LINKER_FLAGS="$COVERAGE_FLAGS" \
  -DCMAKE_INSTALL_RPATH="$TEST_PREFIX/lib"
cmake --build "$M/build-test" -j"$MAYHEM_JOBS"
cmake --install "$M/build-test" >/dev/null

# 3) Harnesses: each linked twice (libFuzzer + run-once standalone reproducer).
HFLAGS=($SANITIZER_FLAGS $DEBUG_FLAGS -O1 -I"$FUZZ_PREFIX/include/gmt" -I"$SRC/src" -I"$M/harnesses"
        -DGMT_FUZZ_PLUGIN="\"$FUZZ_PREFIX/lib/gmt/plugins/supplements.so\"")
LIBS=(-L"$FUZZ_PREFIX/lib" -lgmt -Wl,-rpath,"$FUZZ_PREFIX/lib" -lm -ldl)
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$M/lsan_off.c" -o "$M/out/lsan_off.o"
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$M/out/standalone_main.o"

for h in "$M"/harnesses/fuzz_*.c "$M"/harnesses/fuzz_*.cpp; do
  [ -e "$h" ] || continue
  name="$(basename "${h%.*}")"
  case "$h" in *.cpp) cc="$CXX" ;; *) cc="$CC" ;; esac
  $cc "${HFLAGS[@]}" $LIB_FUZZING_ENGINE "$h" "$M/out/lsan_off.o" "${LIBS[@]}" -o "$BIN/$name"
  $cc "${HFLAGS[@]}" "$h" "$M/out/standalone_main.o" "$M/out/lsan_off.o" "${LIBS[@]}" -o "$BIN/$name-standalone"
done
echo "built: $(ls "$BIN" | tr '\n' ' ')"
