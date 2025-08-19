# AGENTS

## Scope
These instructions apply to the entire repository unless a more specific `AGENTS.md` exists in a subdirectory.

## Workflow
- Ensure your branch is up to date with the target branch by rebasing or merging before making changes and again before committing or submitting a PR to avoid merge conflicts when multiple PRs are in progress.
- Follow the existing code style for Kotlin, C++, and build scripts.
- Keep commits focused and easy to review.
- Use clear, imperative commit messages (e.g., `Add feature X`).

## Testing
- Run `./gradlew test` before committing to execute unit tests and basic checks.
- If you modify native code or build scripts, also run `./gradlew assembleDebug` to ensure the project builds.
- Resolve all test or build failures before committing. If a command cannot run, mention the reason in your PR.

## Documentation
- Update README files and comments whenever behavior changes or new features are added.

## Pull Requests
- Provide a concise summary of changes.
- Reference relevant files and include test output in the PR description.

