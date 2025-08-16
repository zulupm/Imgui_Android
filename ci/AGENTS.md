# CI Scripts

- Run `record_ci_error.sh` to reproduce CI build issues locally.
- The script updates git submodules to mirror the CI environment and logs system info in `last_ci_error.log`.
- Commit `last_ci_error.log` when a build fails for easier debugging.
