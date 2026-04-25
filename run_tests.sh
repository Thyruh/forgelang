#!/bin/bash

FORGE=./forge
PASS=0
FAIL=0

RED='\033[0;31m'
GREEN='\033[0;32m'
RESET='\033[0m'

for test_dir in tests/*/; do
  test_dir="${test_dir%/}"
  input="$test_dir/input.fg"
  expected="$test_dir/expected.txt"

  if [ ! -f "$input" ] || [ ! -f "$expected" ]; then
    continue
  fi

  actual=$($FORGE "$input" 2>&1 | grep -v "^rm:")
  diff_output=$(diff <(echo "$actual") "$expected")

  if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${RESET}: $test_dir"
    PASS=$((PASS + 1))
  else
    echo -e "${RED}FAIL${RESET}: $test_dir"
    echo "$diff_output"
    FAIL=$((FAIL + 1))
  fi
done

echo ""
echo "Results: $PASS passed, $FAIL failed"
if [ $FAIL -gt 0 ]; then
  exit 1
fi
