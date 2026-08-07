#!/usr/bin/env bash
# run_tests.sh — runs every file in <lang>/tests/ through the built binary
# under valgrind (full leak check) and reports TOTAL / FAILED.
# A test only counts as FAILED on a memory error (leak, invalid read/write,
# double-free) — a compiler correctly rejecting a bad test file (exit 1) is
# NOT a failure, it's the expected behavior for that test case.
#
# Usage: ./run_tests.sh   (run from repo root; builds each frontend if needed)

set -uo pipefail

LANGS=(c cpp java python)
declare -A BIN=( [c]=mm_c [cpp]=mm_cpp [java]=mm_java [python]=mm_python )

if ! command -v valgrind >/dev/null 2>&1; then
  echo "valgrind not found — install it first (e.g. apt-get install valgrind)."
  exit 1
fi

TOTAL=0
FAILED=0
FAIL_LIST=()

for lang in "${LANGS[@]}"; do
  bin="$lang/${BIN[$lang]}"
  if [ ! -x "$bin" ]; then
    echo "Building $lang..."
    (cd "$lang" && make -s) || { echo "BUILD FAILED: $lang"; exit 1; }
  fi
  for f in "$lang"/tests/*; do
    TOTAL=$((TOTAL+1))
    valgrind --leak-check=full --error-exitcode=99 --quiet "$bin" "$f" > /dev/null 2>&1
    code=$?
    if [ "$code" -eq 99 ]; then
      FAILED=$((FAILED+1))
      FAIL_LIST+=("$f")
    fi
  done
done

echo
echo "TOTAL=$TOTAL  FAILED=$FAILED"
if [ "$FAILED" -gt 0 ]; then
  echo "Failed (memory errors):"
  printf '  %s\n' "${FAIL_LIST[@]}"
  exit 1
fi
exit 0
