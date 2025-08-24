# AGENTS

## Scope
These instructions apply to the entire repository unless a more specific `AGENTS.md` exists in a subdirectory.

## Workflow
- Check for `AGENTS.md` files in any directories you touch and follow their instructions.
- Ensure your branch is up to date with the target branch before and after making changes to avoid conflicts.
- Use `rg` for searching the codebase and avoid `ls -R` or `grep -R`.
- Follow the existing code style for Kotlin, C++, and build scripts.
- Keep commits focused and easy to review with clear, imperative messages (e.g., `Add feature X`).

## Environment
- Install the Android SDK and NDK and set `sdk.dir` and `ndk.dir` in `local.properties`.
- Use NDK version `25.2.9519653` for builds.
- Export `ANDROID_NDK_HOME`/`ANDROID_NDK_ROOT` so build scripts can locate the toolchain.
- Android's NDK ships BoringSSL, so no external OpenSSL build is required for TLS support.
- The demo relies on `network_security_config.xml` and Internet permissions for secure WebSocket connections; keep these enabled.

## Testing
- Run `./gradlew test` before committing to execute unit tests and basic checks.
- If you modify native code or build scripts, also run `./gradlew assembleDebug` to ensure the project builds.
- Resolve all test or build failures before committing. If a command cannot run, mention the reason in your PR.

## Documentation
- Update README files and comments whenever behavior changes or new features are added.

## Pull Requests
- Provide a concise summary of changes.
- Reference relevant files and include test output in the PR description.

