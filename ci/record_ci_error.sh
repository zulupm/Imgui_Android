#!/usr/bin/env bash
set -o pipefail
LOG_FILE="$(dirname "$0")/last_ci_error.log"

# Ensure submodules match the development environment
git submodule update --init --recursive

# Record environment details
{
  echo "System info:"
  uname -a
  echo "cmake version:"
  cmake --version
  echo ""
  echo "---- Build output ----"
} > "$LOG_FILE"

# Run the build and tee output to log
if ! cmake -S ImguiDemoSdl/src/main/cpp -B build/test-build "$@" 2>&1 | tee -a "$LOG_FILE"; then
  echo "Build failed; log written to $LOG_FILE" >&2
  # Attempt to commit the log for later inspection
  if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    git add "$LOG_FILE"
    git commit -m "chore: record CI build failure" || true
  fi
  exit 1
fi
