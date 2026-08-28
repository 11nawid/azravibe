# Security Policy

## Reporting a Vulnerability

If you discover a security vulnerability in Azravibe, please report it responsibly.

**Do not open a public GitHub issue for security vulnerabilities.**

Instead, please report security issues through [GitHub's private vulnerability reporting](https://github.com/azravibe/azravibe/security/advisories/new) if available, or by contacting the maintainer through a private channel.

### What to Include

When reporting a vulnerability, please include:

1. A description of the vulnerability
2. Steps to reproduce the issue
3. The potential impact
4. Any suggested fixes (if applicable)

### Response Timeline

- **Acknowledgment:** Within 48 hours of your report
- **Assessment:** Within 1 week
- **Fix or mitigation:** Depends on severity; critical issues will be prioritized

### What to Expect

- Your report will be treated confidentially
- You will not be penalized for reporting a vulnerability
- Credit will be given to reporters (unless they prefer anonymity)
- You will be notified when the issue is fixed

## Scope

This security policy applies to:

- The `azravibe` language interpreter
- The Azravibe IDE
- The native extension API

This policy does **not** apply to:

- Code written by users in Azravibe (user programs)
- Third-party native extensions
- Third-party dependencies (report those to their maintainers)

## Security Considerations

Azravibe is early-stage software. Users should be aware of the following:

- The interpreter executes arbitrary code -- only run `.azr` files you trust
- The `system`/`فرمان` builtin executes shell commands
- The HTTP module shells out to `curl`
- Native extensions have full access to the C API
- There is no sandboxing of user code

## Supported Versions

| Version | Supported |
|---|---|
| v.0.1 | Yes |

## Best Practices

When using Azravibe:

- Do not run untrusted `.azr` files without reviewing them first
- Be cautious with native extensions from unknown sources
- Do not pass user input directly to `system`/`فرمان` without validation
- Use the HTTP module with awareness that it makes real network requests
