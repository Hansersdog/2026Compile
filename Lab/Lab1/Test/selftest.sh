#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/../Code"

make -s clean
make -s

for f in ../Test/test1.cmm ../Test/test2.cmm ../Test/lex_err.cmm ../Test/num_ext.cmm ../Test/comment_err.cmm ../Test/syntax_err.cmm; do
  echo "===== $f ====="
  ./parser "$f"
done
