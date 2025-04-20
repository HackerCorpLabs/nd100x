# Contributing to nd100x

Thank you for your interest in contributing to **nd100x**, a modernized ND-100/CX emulator based on the original `nd100em` project!

We welcome contributions of all kinds — code, documentation, testing, issue reporting, or general feedback. Whether you're a seasoned emulator developer or just getting started with retro computing, your help is appreciated.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How to Contribute](#how-to-contribute)
- [Bug Reports and Feature Requests](#bug-reports-and-feature-requests)
- [Code Style and Guidelines](#code-style-and-guidelines)
- [Build and Test](#build-and-test)
- [Commit Messages](#commit-messages)
- [License](#license)

---

## Code of Conduct

Please be respectful and constructive. We follow the principles of [Contributor Covenant](https://www.contributor-covenant.org/version/2/1/code_of_conduct/). Be kind to others and contribute in the spirit of learning and collaboration.

---

## How to Contribute

1. **Fork the repository** and clone it locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/nd100x.git
   cd nd100x
   ```

2. **Create a new branch** for your feature or fix:
   ```bash
   git checkout -b feature/my-awesome-change
   ```

3. **Make your changes** locally. Try to keep changes focused and minimal.

4. **Test your changes** (see [Build and Test](#build-and-test) below).

5. **Commit** with a clear message (see [Commit Messages](#commit-messages)).

6. **Push to your fork** and open a **pull request** from your branch to the `main` branch of `nd100x`.

---

## Bug Reports and Feature Requests

Use [GitHub Issues](https://github.com/HackerCorpLabs/nd100x/issues) to:

- Report a bug
- Request a new feature or device
- Suggest a documentation improvement
- Track known issues or regressions

Please include:
- A clear title and description
- Reproduction steps (for bugs)
- Platform details (OS, compiler version, etc.)
- Emulator version / Git commit hash

---

## Code Style and Guidelines

- Use `int8_t`, `uint16_t`, etc. for fixed-width types
- Follow C89/C99 style formatting for consistency
- Keep functions short and focused
- Use `//` for inline comments, and `/* ... */` for block comments
- Avoid platform-specific code unless inside `#ifdef` guards
- Don’t include generated files or build output in commits

If unsure, match the style in surrounding code.

---

## Build and Test

This project uses `make` with multiple build modes:

- **Debug build**:
  ```bash
  make debug
  ```

- **Release build**:
  ```bash
  make release
  ```

- **Sanitized build** (for memory issue detection):
  ```bash
  make sanitize
  ```

> 📝 You must have GCC, `make`, and a POSIX-like environment (Linux recommended).

If your change introduces new functionality or fixes a bug, consider adding or updating test programs.

---

## Commit Messages

Use clear and descriptive commit messages. Format:

```
<type>: <short summary>

[optional body]
```

Examples:
```
fix: correct TRAP generation for access violations
feat: add preliminary OPCOM support
docs: fix typo in README
refactor: simplify device dispatch logic
```

---

## License

All contributions to this project are under the same license as the project itself:  
**GNU General Public License v2.0 or later**.

By contributing, you agree that your code may be released under the GPL license.

---

Thank you again for contributing to nd100x! If you have questions or need help, feel free to open an issue or start a discussion.