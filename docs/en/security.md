> **English** | [Français](../fr/security.md)

# Security model

**Babet runs trusted Lua scripts. It is not a sandbox.**

Scripts execute with the full Lua standard library (`luaL_openlibs`),
and Babet additionally exposes filesystem and process primitives
such as `exec`, `remove`, `rmdirAll` and `chdir`. A script therefore
has the same privileges as the process running it : it can read,
modify or delete files and run arbitrary commands. This is by
design — like `make`, a shell script, or any build tool, Babet is
meant to run code you control.

Only run `main.lua` and `require`d modules that you trust. Do **not**
use Babet to execute Lua from untrusted sources ; it provides no
isolation against hostile code, and none is intended. If you need to
run untrusted scripts, use a purpose-built Lua sandbox instead.

## What the hardening *does* protect against

The hardening work in Babet protects *legitimate* use against
accidents and supply-chain tampering :

- **Path / symlink handling** uses `lexically_relative()` and
  component-wise `is_within()` guards (never string prefix checks),
  so a `cp src dst/` cannot escape into a sibling tree via a
  symlinked path component.
- **Process-group cleanup** in `babet.exec` ensures that
  forked children don't outlive the parent or leak zombie
  processes when the script is interrupted.
- **Output limits** on `babet.exec` bound the amount of stdout
  and stderr captured into memory (default 16 MiB each, configurable),
  so a runaway subprocess can't OOM the Babet process.
- **Dependency checksums** : every vendored dependency is
  SHA256-pinned in `build_local.sh`. An empty hash refuses to build.
  A mismatch deletes the downloaded file and exits with an error.
  This protects against a compromised upstream or Wayback Machine
  archive (which is the fallback source).
- **TLS verification** is on by default (`verify=true`). Disabling
  it requires an explicit `verify=false` per call — no silent
  fallback. Custom CA stores can be passed via `ca_cert` or
  `ca_path`, or via `SSL_CERT_FILE` / `SSL_CERT_DIR` environment
  variables.

## What it does *not* protect against

- A malicious script. Babet has no sandbox. If you `exec` user
  input, you have a shell injection. If you `loadstring` user
  input, you have arbitrary code execution.
- A determined attacker who has obtained code execution on the
  machine running Babet. The hardening makes accidental
  mistakes loud, not adversarial attacks impossible.
- Long-tail OS-level issues (kernel exploits, container escapes,
  privilege escalation). Babet is an ordinary userland binary.

## Recommended pattern : least privilege

When running Babet as a service, treat it like any other
unprivileged process :

```sh
# As root, create a dedicated service user with no shell.
useradd --system \
        --home-dir /var/lib/myapp \
        --create-home \
        --shell /usr/sbin/nologin \
        --comment "myapp Babet service" \
        myapp

# Use babet.user.exists("myapp") in your install script to
# verify the user is provisioned before launching the daemon.
```

Combine with `systemd` unit file options like `User=myapp`,
`PrivateTmp=yes`, `ProtectSystem=strict`, `NoNewPrivileges=yes`,
and `CapabilityBoundingSet=` (empty unless you genuinely need a
capability). The kernel does much more than Babet can to
contain a misbehaving script ; let it.
