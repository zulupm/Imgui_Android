# AGENTS

## Scope
These instructions apply to the entire repository unless a more specific `AGENTS.md` exists in a subdirectory.

## Workflow
- Check for `AGENTS.md` files in any directories you touch and follow their instructions.
- Ensure your branch is up to date with the target branch before and after making changes to avoid conflicts.
- Use `rg` for searching the codebase and avoid `ls -R` or `grep -R`.
- Follow the existing code style for Kotlin, C++, and build scripts.
- Keep commits focused and easy to review with clear, imperative messages (e.g., `Add feature X`).
- Once a technical direction is chosen (e.g., using the NDK's BoringSSL for TLS), do not switch to alternative stacks unless explicitly requested by maintainers.
- Investigate and resolve build or CI failures immediately rather than committing temporary workarounds.

## Environment
- Install the Android SDK and NDK and set `sdk.dir` and `ndk.dir` in `local.properties`.
- Use NDK version `25.2.9519653` for builds.
- Ensure the NDK path in `local.properties` matches `android.ndkVersion` to avoid version conflicts.
- Export `ANDROID_SDK_ROOT` and `ANDROID_NDK_HOME`/`ANDROID_NDK_ROOT` so build scripts and CI workflows can locate the toolchains consistently.
- Link against the NDK's bundled TLS libraries (BoringSSL); if TLS libraries are reported missing, fix the environment instead of disabling secure WebSockets.
- Include BoringSSL headers from `${ANDROID_NDK}/sources/third_party/openssl/include` so native code can find `openssl/ssl.h`.
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

