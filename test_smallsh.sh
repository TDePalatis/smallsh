#!/usr/bin/env bash
# Automated tests for smallsh
# This script feeds commands into ./smallsh and checks the output/side effects.
# It assumes:
#   - You compiled the shell as: gcc -Wall -Wextra -std=c99 -o smallsh smallsh.c
#   - You run this script from the same directory as smallsh

set -euo pipefail

BIN=./smallsh

if [[ ! -x "$BIN" ]]; then
  echo "ERROR: $BIN not found or not executable."
  echo "Compile first with:"
  echo "  gcc -Wall -Wextra -std=c99 -o smallsh smallsh.c"
  exit 1
fi

pass=0
fail=0

run_test() {
  local name="$1"
  local script="$2"
  local expect="$3"

  echo "Running: $name"

  # Feed scripted input into smallsh.
  # We allow smallsh to exit with non-zero without killing this script (hence '|| true').
  output="$(
    printf "%s\n" "$script" | "$BIN" 2>&1 || true
  )"

  if [[ -n "$expect" ]]; then
    if grep -qE "$expect" <<<"$output"; then
      echo "  ✔ PASS"
      ((pass++))
    else
      echo "  ✘ FAIL"
      echo "    Expected to match regex: $expect"
      echo "    Actual output:"
      echo "------------------------"
      echo "$output"
      echo "------------------------"
      ((fail++))
    fi
  else
    # No explicit expectation – just reaching here counts as pass
    echo "  ✔ PASS (no explicit expectation)"
    ((pass++))
  fi
}

#####################
# Individual tests  #
#####################

# 1. 'status' on fresh start should report exit value 0
run_test \
  "status initial" \
  $'status\nexit' \
  'exit value 0'

# 2. Unknown command should print an error
run_test \
  "unknown command" \
  $'thiscommanddoesnotexist\nexit' \
  'thiscommanddoesnotexist: no such file or directory'

# 3. Foreground external command via redirection (ls -> ls_out.txt)
#    We don't rely on the exact text of ls; we just check the file was created and is non-empty.
LS_OUT="ls_out.txt"
rm -f "$LS_OUT"

run_test \
  "foreground ls with redirection" \
  $'ls > ls_out.txt\nexit' \
  ''

if [[ -s "$LS_OUT" ]]; then
  echo "  ✔ PASS (ls_out.txt created and non-empty)"
  ((pass++))
else
  echo "  ✘ FAIL (ls_out.txt missing or empty)"
  ((fail++))
fi

rm -f "$LS_OUT"

# 4. Background process should print a background pid line
run_test \
  "background sleep" \
  $'sleep 1 &\nexit' \
  'background pid is [0-9]+'

# 5. Redirection: sort input to output file
TMP_IN="test_in.txt"
TMP_OUT="test_out.txt"
printf "c\nb\na\n" > "$TMP_IN"

run_test \
  "redirection sort command" \
  $'sort < test_in.txt > test_out.txt\nexit' \
  ''

if [[ -f "$TMP_OUT" ]] && diff <(printf "a\nb\nc\n") "$TMP_OUT" >/dev/null 2>&1; then
  echo "  ✔ PASS (redirection sort output correct)"
  ((pass++))
else
  echo "  ✘ FAIL (sorted output incorrect or file missing)"
  ((fail++))
fi

rm -f "$TMP_IN" "$TMP_OUT"

#####################
# Summary           #
#####################

echo
echo "=============================="
echo "Tests passed:   $pass"
echo "Tests failed:   $fail"
echo "=============================="

if [[ "$fail" -ne 0 ]]; then
  exit 1
fi
