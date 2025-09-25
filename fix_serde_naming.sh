#!/usr/bin/env bash
set -euo pipefail

echo "==> Checking git repo root"
test -d .git || { echo "Run this in the repo root (where .git exists)"; exit 1; }

# Skapa/byt till feature-branch
if git rev-parse --verify feat/lib_serde-v0 >/dev/null 2>&1; then
  echo "==> Switching to existing branch feat/lib_serde-v0"
  git switch feat/lib_serde-v0
else
  echo "==> Creating branch feat/lib_serde-v0"
  git switch -c feat/lib_serde-v0
fi

# 1) Mapp: lib_cerde -> lib_serde
if [ -d lib_cerde ]; then
  echo "==> git mv lib_cerde -> lib_serde"
  git mv lib_cerde lib_serde
fi

# 2) Filer: cerde.h/.cpp -> serde.h/.cpp
if [ -f lib_serde/cerde.h ]; then
  echo "==> git mv lib_serde/cerde.h -> lib_serde/serde.h"
  git mv lib_serde/cerde.h lib_serde/serde.h
fi
if [ -f lib_serde/cerde.cpp ]; then
  echo "==> git mv lib_serde/cerde.cpp -> lib_serde/serde.cpp"
  git mv lib_serde/cerde.cpp lib_serde/serde.cpp
fi

# 3) Exempel: examles -> examples, cerde_print -> serde_print
if [ -d lib_serde/examles ]; then
  echo "==> git mv lib_serde/examles -> lib_serde/examples"
  git mv lib_serde/examles lib_serde/examples
fi
if [ -d lib_serde/examples/cerde_print ]; then
  echo "==> git mv lib_serde/examples/cerde_print -> lib_serde/examples/serde_print"
  git mv lib_serde/examples/cerde_print lib_serde/examples/serde_print
fi

# 4) Ersätt text: 'cerde' -> 'serde' i innehåll under lib_serde/
echo '==> Replacing text "cerde" -> "serde" in file contents'
git ls-files -co --exclude-standard -- lib_serde | xargs -r sed -i 's/cerde/serde/g'

# 5) Visa vad som ändrats och committa
echo "==> Staging changes"
git add -A
echo "==> git status:"
git status

echo "==> Committing"
git commit -m "fix: rename lib_cerde->lib_serde; cerde->serde; fix examples paths"

echo "==> Pushing branch feat/lib_serde-v0"
git push -u origin feat/lib_serde-v0

echo "==> Done. Open the PR link printed by Git above."
