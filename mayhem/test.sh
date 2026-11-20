#!/usr/bin/env bash
#
# mayhem/test.sh — RUN this repo's OWN functional test suite (already built by mayhem/build.sh).
# exit 0 = pass. EDIT per repo. PATCH-grade oracle: after an agent patches the source, the grader
# rebuilds (build.sh) then runs this. DELETE this file if the repo has no meaningful tests.
#
# IMPORTANT:
#  * Must assert BEHAVIOR/OUTPUT, not just exit status. The oracle has to check asserted values /
#    golden-output diffs / known-answer results — so a PATCH that "fixes" a bug by making the program
#    exit(0) (or any no-op) FAILS here. Running inputs and checking only "exit 0 / didn't crash" is
#    NOT a functional test (it's trivially reward-hackable) — use the project's real assertion suite.
#  * Do NOT build here — mayhem/build.sh already compiled the test suite (with the project's normal
#    flags). This script only RUNS the pre-built tests and reports counts. If the test runner is
#    missing, that's a build.sh bug — fail loudly rather than silently rebuilding.
#  * REQUIRED OUTPUT — a CTRF (https://ctrf.io) summary so Mayhem/the PATCH grader reads the counts:
#      - writes a CTRF JSON report to ${CTRF_REPORT:-$SRC/ctrf-report.json}, and
#      - prints a one-line `CTRF {...}` marker to stdout (same JSON, compact).
#    Only `results.summary` (with tests/passed/failed/pending/skipped/other) is required.
#    Use the emit_ctrf helper below; it computes tests = passed+failed+skipped and sets the exit
#    code (0 iff failed==0). Map your framework's output to passed/failed/skipped.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${MAYHEM_JOBS:=$(nproc)}"   # build parallelism; env-overridable, falls back to nproc (use -j"$MAYHEM_JOBS")
cd "$SRC"

# emit_ctrf <tool> <passed> <failed> [skipped] [pending] [other]
# Writes a CTRF report (file + stdout `CTRF {...}` marker) and returns non-zero iff failed>0.
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

# Known-answer checks against the normal-flags GMT build (mayhem/out/test), covering the readers and
# interpreters the fuzz targets exercise: RPN math, grid write/read, table I/O, KML, projections,
# CPT generation, MGD77 header parsing, geodesic distances and the defaults system.
GMT="$SRC/mayhem/out/test/bin/gmt"
[ -x "$GMT" ] || { echo "missing $GMT — mayhem/build.sh did not build the test tree" >&2; emit_ctrf gmt-known-answer 0 1; exit 1; }
WORK="$(mktemp -d "${TMPDIR:-/tmp}/gmt-test.XXXXXX")"
trap 'rm -rf "$WORK"' EXIT
export HOME="$WORK" GMT_USERDIR="$WORK" GMT_TMPDIR="$WORK"
cd "$WORK"
TAB=$'\t'
pass=0; fail=0
check() {  # check <name> <expected> <command...>
  local name="$1" expected="$2"; shift 2
  local got; got="$("$@" 2>/dev/null)"
  if [ "$got" == "$expected" ]; then pass=$((pass+1)); echo "PASS $name"
  else fail=$((fail+1)); echo "FAIL $name"; echo "  expected: $(printf %q "$expected")"; echo "  got:      $(printf %q "$got")"; fi
}
printf '1 2 3\n4 5 6\n' > t.txt
check gmtmath "0${TAB}0
1${TAB}2
2${TAB}4
3${TAB}6
4${TAB}8" "$GMT" math -T0/4/1 T 2 MUL =
"$GMT" grdmath -R0/3/0/3 -I1 X Y MUL = g.nc 2>/dev/null
check grdinfo "g.nc${TAB}0${TAB}3${TAB}0${TAB}3${TAB}0${TAB}9${TAB}1${TAB}1${TAB}4${TAB}4${TAB}0${TAB}0" "$GMT" grdinfo -C g.nc
check grd2xyz "$(printf '0\t3\t0\n1\t3\t3\n2\t3\t6\n3\t3\t9\n0\t2\t0')" bash -c "\"$GMT\" grd2xyz g.nc | head -5"
"$GMT" grdconvert g.nc g.bf=bf 2>/dev/null
check grd_native_bf "$(printf '3\t3\t9')" bash -c "\"$GMT\" grd2xyz g.bf=bf | sed -n 4p"
check gmtconvert "$(printf '3\t1\n6\t4')" "$GMT" convert t.txt -o2,0
check kml2gmt "$(printf '> -L"Line 0"\n0\t0\n0\t1\n> -L"Line 1"')" bash -c "\"$GMT\" kml2gmt \"$SRC/test/kml/lines.kml\" | head -4"
check mapproject "3.33333333333${TAB}3.32818357603" bash -c "echo 10 10 | \"$GMT\" mapproject -R0/30/0/30 -JM10c"
check geodesic "1${TAB}1${TAB}156.898594962" bash -c "echo 1 1 | \"$GMT\" mapproject -G0/0+uk"
check makecpt "$(printf '0\tgray25\t0.5\tgray25\tL\n0.5\tgray75\t1\tgray75\tB\nB\tblack\nF\twhite\nN\t128')" "$GMT" makecpt -Cgray -T0/1/0.5
check mgd77info "01010221${TAB}  RC2308${TAB}-159.5196${TAB}-157.0708${TAB}18.956${TAB}24.5089${TAB}1982-08-13T01:09:00${TAB}1982-09-07T17:02:00${TAB}5485${TAB}10178" bash -c "\"$GMT\" mgd77info \"$SRC/test/mgd77/01010221.mgd77\" -E | sed -n 2p | cut -f1-10"
check gmtget "24p,Helvetica,black thicker,black" "$GMT" get FONT_TITLE MAP_FRAME_PEN
printf 'FONT_TITLE = 13p,Times-Bold,red\n' > my.conf
check gmtget_file "13p,Times-Bold,red" "$GMT" get -Gmy.conf FONT_TITLE

emit_ctrf gmt-known-answer "$pass" "$fail" 0
