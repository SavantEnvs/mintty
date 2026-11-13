#!/usr/bin/env bash
#
# mayhem/test.sh — runs the known-answer selftest that mayhem/build.sh built (no compiling here).
# It drives mintty's real terminal core with escape sequences and checks the resulting screen cells,
# plus base64, sixel and bidi known answers. Counts come from its own PASS/FAIL lines.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SRC:=/mayhem}"
cd "$SRC"

emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}

runner="$SRC/mintty_selftest"
if [ ! -x "$runner" ]; then
  echo "mayhem/test.sh: $runner missing — mayhem/build.sh should have built it" >&2
  emit_ctrf mintty-selftest 0 1
  exit 1
fi

out="$("$runner" 2>&1)"
rc=$?
printf '%s\n' "$out"
passed="$(printf '%s\n' "$out" | grep -c '^PASS ' || true)"
failed="$(printf '%s\n' "$out" | grep -c '^FAIL ' || true)"
summary="$(printf '%s\n' "$out" | sed -n 's/^SUMMARY: \([0-9]*\) passed, \([0-9]*\) failed$/\1 \2/p')"
if [ -z "$summary" ] || [ "$summary" != "$passed $failed" ] || [ "$passed" -eq 0 ]; then
  failed=$(( failed + 1 ))
elif [ "$rc" -ne 0 ] && [ "$failed" -eq 0 ]; then
  failed=1
fi
emit_ctrf mintty-selftest "$passed" "$failed"
