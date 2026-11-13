#!/usr/bin/env bash
#
# mayhem/build.sh — builds mintty's headless terminal core (escape-sequence parser, sixel, base64, bidi)
# into fuzz harnesses, their -standalone reproducers, and a known-answer selftest for mayhem/test.sh.
#
# mintty is a Windows/Cygwin terminal, so it does not build on Linux as shipped. The code under test is
# compiled unmodified: mayhem/shim/ stands in for the Windows headers and for the 16-bit wide-string libc
# calls (mintty's `wchar` is UTF-16), and mayhem/stubs.c replaces the GUI/pty half. src/termout.c uses GCC
# nested functions, so the core is built with gcc (not clang); gcc's -fsanitize-coverage=trace-pc callback is
# renamed to mt_cov_trace_pc, which feeds libFuzzer's inline 8-bit counters (mayhem/shim/gcc_cov.c).
set -euo pipefail

[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${STANDALONE_FUZZ_MAIN:=/opt/mayhem/StandaloneFuzzTargetMain.c}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

SRC="${SRC:-/mayhem}"
cd "$SRC"

B="$SRC/mayhem/build"
rm -rf "$B"
mkdir -p "$B/fuzz" "$B/test"

CORE="termline termclip termmouse mcwidth minibidi base64 sixel sixel_hls"
INC="-include $SRC/mayhem/shim/mt16.h -include std.h -I$SRC/mayhem/shim -I$SRC/src -I$SRC/mayhem"
WARN="-w -std=gnu99"

# build_core <outdir> <flags...>: compile the mintty core + shims with gcc; term.c and termout.c (through the
# mayhem/term_wrap.c and mayhem/termout_wrap.c wrappers, which add per-input resets of their file-statics).
# termout.c gets two sanitizer relaxations gcc needs for it (shift-base: its `1 << 31` case labels stop
# being constant expressions; use-after-scope: gcc poisons the nested-function frame object while it is
# still in use).
build_core() {
  local out="$1"; shift
  local relax="$1"; shift
  local objs=()
  for f in $CORE; do
    gcc "$@" $INC $WARN -c "src/$f.c" -o "$out/$f.o" &
  done
  gcc "$@" $INC $WARN -c mayhem/term_wrap.c -o "$out/term.o" &
  gcc "$@" $relax $INC $WARN -c mayhem/termout_wrap.c -o "$out/termout.o" &
  gcc "$@" $INC $WARN -c mayhem/stubs.c -o "$out/stubs.o" &
  gcc "$@" $INC $WARN -c mayhem/shim/mt16.c -o "$out/mt16.o" &
  wait
}

# 1) The sanitized, SanCov-instrumented core, with DWARF <= 3.
FUZZ_CC_FLAGS="-O1 $SANITIZER_FLAGS $DEBUG_FLAGS -fsanitize-coverage=trace-pc"
RELAX="-fno-sanitize=shift-base -fno-sanitize-address-use-after-scope"
# shellcheck disable=SC2086
build_core "$B/fuzz" "$RELAX" $FUZZ_CC_FLAGS
gcc -O1 $DEBUG_FLAGS -c mayhem/shim/gcc_cov.c -o "$B/fuzz/gcc_cov.o"
# libFuzzer rejects gcc's trace-pc callback; point it at the shim instead.
for o in "$B"/fuzz/*.o; do
  [ "$o" = "$B/fuzz/gcc_cov.o" ] || objcopy --redefine-sym __sanitizer_cov_trace_pc=mt_cov_trace_pc "$o"
done
rm -f "$B/fuzz/libmintty_core.a"
ar rcs "$B/fuzz/libmintty_core.a" "$B"/fuzz/*.o

# LeakSanitizer off, at build time, for every fuzz and standalone binary.
# shellcheck disable=SC2086
"$CXX" $SANITIZER_FLAGS $DEBUG_FLAGS -c mayhem/lsan_off.cc -o "$B/fuzz/lsan_off.o"
# shellcheck disable=SC2086
"$CC" $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$B/fuzz/standalone_main.o"

# 2) One libFuzzer binary and one run-once reproducer per harness.
for t in fuzz_term fuzz_sixel fuzz_base64 fuzz_bidi; do
  # shellcheck disable=SC2086
  gcc $FUZZ_CC_FLAGS $INC $WARN -c "mayhem/$t.c" -o "$B/fuzz/$t.o"
  objcopy --redefine-sym __sanitizer_cov_trace_pc=mt_cov_trace_pc "$B/fuzz/$t.o"
  # shellcheck disable=SC2086
  "$CC" $SANITIZER_FLAGS $DEBUG_FLAGS $LIB_FUZZING_ENGINE "$B/fuzz/$t.o" "$B/fuzz/libmintty_core.a" \
      "$B/fuzz/lsan_off.o" -o "$SRC/$t"
  # shellcheck disable=SC2086
  "$CC" $SANITIZER_FLAGS $DEBUG_FLAGS "$B/fuzz/$t.o" "$B/fuzz/standalone_main.o" "$B/fuzz/libmintty_core.a" \
      "$B/fuzz/lsan_off.o" -o "$SRC/$t-standalone"
done

# 3) The known-answer selftest, with the project's normal flags (no sanitizers, no -gdwarf-3).
# shellcheck disable=SC2086
build_core "$B/test" "" -O2 $COVERAGE_FLAGS
# shellcheck disable=SC2086
gcc -O2 $COVERAGE_FLAGS $INC $WARN mayhem/selftest.c "$B"/test/*.o -o "$SRC/mintty_selftest"
