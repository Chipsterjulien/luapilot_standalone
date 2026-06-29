> **English** | [Français](../fr/README.md)

<p align="center">
  <img src="../assets/babet-closed.png" alt="Babet" width="160">
</p>

# Babet — User Manual

> *Babet*, n.m. — regional word from south-eastern France
> (Lyonnais, Forez, Dauphiné, Savoie, and adjacent Swiss
> Romandy) for a pine cone. Small, light, packed with seeds,
> and good for starting a fire — like this binary.

Babet is a standalone Lua 5.5 binary for Linux scripting and
automation. This is the reference manual, organised one file per
module so each section stays focused and easy to maintain.

## Getting started

- [`getting-started.md`](getting-started.md) — install, first
  script, the three launch modes (folder / embedded / via PATH).
- [`security.md`](security.md) — what the hardening does and
  doesn't protect against, recommended least-privilege patterns.

## Modules

Each module lives in its own file under [`modules/`](modules/):

| Module | Purpose |
| --- | --- |
| [`argparse`](modules/argparse.md) | Command-line argument parser. |
| [`exec`](modules/exec.md) | Run external programs and capture output. |
| [`fs`](modules/fs.md) | File system : list, copy, hash, attrs. |
| [`http`](modules/http.md) | HTTP client (built on cpp-httplib). |
| [`inotify`](modules/inotify.md) | Filesystem event watching (`inotify(7)`). |
| [`json`](modules/json.md) | JSON encode/decode (nlohmann/json). |
| [`logging`](modules/logging.md) | Leveled logger. |
| [`signal`](modules/signal.md) | POSIX signals : SIGTERM, SIGINT, … |
| [`socket`](modules/socket.md) | TCP client/server with timeouts. |
| [`sqlite`](modules/sqlite.md) | Embedded SQL database. |
| [`strings`](modules/strings.md) | String manipulation (`split`, etc.). |
| [`sys`](modules/sys.md) | `which`, `hostname`, `env`, `pid`, … |
| [`tables`](modules/tables.md) | Table manipulation (`mergeTables`, `deepCopyTable`). |
| [`time`](modules/time.md) | Monotonic and realtime clocks. |
| [`tls`](modules/tls.md) | TLS sockets : `connect_tls`, `starttls`. |
| [`toml`](modules/toml.md) | TOML parser (tomlplusplus). |
| [`user`](modules/user.md) | System user lookups via NSS. |
| [`workers`](modules/workers.md) | Parallel jobs (real OS threads). |

The [`user`](modules/user.md) module is the canonical reference for
the documentation pattern : *Why*, *API*, *Quick example*, *Error
contract*, *Design decisions*, *Not in v1*. Other modules vary in
how strictly they follow that template ; the goal over time is to
align them.

## Cookbook

[`cookbook.md`](cookbook.md) — concrete recipes : directory watch +
email, parallel HTTP fetches, graceful shutdown, TOML+SQLite, etc.

## Building a PDF

The manual can be exported to a single PDF :

```sh
cd docs
./build_doc.sh         # uses pandoc + LaTeX
```

See [`docs/manual_order_en.txt`](../manual_order_en.txt) for the
file order ; that's the only place to edit when adding or
reordering chapters.

## Conventions used in the docs

- **API tables** : every module starts with a small table listing
  functions, signatures, and what they return.
- **Error contract** : a dedicated section explains which errors
  are raised (`luaL_error`) versus returned as `(nil, err)`. This
  is consistent across all modules.
- **Design decisions** : when a choice is non-obvious ("why not
  recursive ?", "why mandatory ?"), there's a short rationale.
- **Not in v1** : a short list of what could be added additively
  later without breaking SemVer.
