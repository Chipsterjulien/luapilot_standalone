> **English** | [Français](../../fr/modules/fs.md)

# `luapilot` fs — file system

A flat set of file system operations exposed directly on `luapilot`
(no sub-namespace, predates the convention). Covers the everyday
needs that Lua's `os.*` and `io.*` don't : recursive listing, copy
with attributes, hashing, attributes, links.

## Why

Lua's standard `io` and `os` give you `open`, `rename`, `remove`,
and that's about it. Everything beyond that — listing a directory,
recursive copy, computing a hash, querying mtime — requires
external programs or low-level FFI. `luapilot` collects these in a
single set of functions that behave consistently (paths are
strings, errors come back as `(nil, "...")`).

All paths are accepted as plain strings. Tilde expansion (`~/x`)
is **not** automatic — the caller must resolve `$HOME` itself if
needed. Functions never expand globs ; for glob support, use
`luapilot.find` with an explicit pattern.

## API — Existence, type, attributes

| Function | Returns |
| --- | --- |
| `luapilot.fileExists(path)` | `boolean` |
| `luapilot.isfile(path)` | `boolean` (true only for regular files) |
| `luapilot.isdir(path)` | `boolean` |
| `luapilot.fileSize(path)` | `integer` bytes \| `(nil, err)` |
| `luapilot.attributes(path)` | `table` with `mtime`, `mode`, `size`, `uid`, `gid`, `inode`, etc. \| `(nil, err)` |
| `luapilot.symlinkattr(path)` | same shape as `attributes`, but `lstat` (does not follow symlinks) \| `(nil, err)` |
| `luapilot.mode(path)` | octal `integer` \| `(nil, err)` — shortcut for `attributes(path).mode` |

## API — Creating, removing, moving

| Function | Returns |
| --- | --- |
| `luapilot.touch(path)` | `(true, nil)` \| `(nil, err)` — create file or update mtime |
| `luapilot.mkdir(path, opts?)` | `(true, nil)` \| `(nil, err)` — `opts.parents = true` for `mkdir -p` |
| `luapilot.rmdir(path)` | `(true, nil)` \| `(nil, err)` — only empty dirs |
| `luapilot.rmdirAll(path)` | `(true, nil)` \| `(nil, err)` — recursive |
| `luapilot.remove(path)` | `(true, nil)` \| `(nil, err)` — file or symlink |
| `luapilot.rename(old, new)` | `(true, nil)` \| `(nil, err)` |
| `luapilot.chdir(path)` | `(true, nil)` \| `(nil, err)` |
| `luapilot.currentDir()` | `string` (absolute) |
| `luapilot.joinPath(a, b, ...)` | `string` — like `path/a/b/c` |
| `luapilot.link(target, link, opts?)` | `(true, nil)` \| `(nil, err)` — `opts.symbolic = true` for symlinks |

## API — Listing and finding

| Function | Returns |
| --- | --- |
| `luapilot.listDir(path)` | `table` (array of entry names, no `.` / `..`) \| `(nil, err)` |
| `luapilot.listFiles(path)` | `table` (only regular files) \| `(nil, err)` |
| `luapilot.find(path, opts)` | iterator function, see below |
| `luapilot.fileIterator(path)` | iterator function over directory entries |

`find` accepts `opts` :

- `pattern = "*.lua"` — shell glob applied to file name only.
- `recursive = true` — descend into sub-directories.
- `type = "f"` / `"d"` — restrict to files or dirs.
- `max_depth = N` — limit recursion depth.

## API — Copy

| Function | Returns |
| --- | --- |
| `luapilot.copy(src, dst, opts?)` | `(true, nil)` \| `(nil, err)` |
| `luapilot.copyTree(src, dst, opts?)` | `(true, nil)` \| `(nil, err)` — recursive |
| `luapilot.moveTree(src, dst)` | `(true, nil)` \| `(nil, err)` |

`copy` options :

- `preserve = true` — keep mtime, mode, owner where possible.
- `overwrite = true` — replace destination if it exists.

## API — Hashes and checksums

Hash digests of file contents. Result is **lowercase hex string**
unless noted.

| Function | Algorithm |
| --- | --- |
| `luapilot.md5(path)` | MD5 |
| `luapilot.sha1(path)` | SHA-1 |
| `luapilot.sha256(path)` | SHA-256 |
| `luapilot.sha384(path)` | SHA-384 |
| `luapilot.sha512(path)` | SHA-512 |
| `luapilot.sha3_256(path)` | SHA3-256 |
| `luapilot.sha3_384(path)` | SHA3-384 |
| `luapilot.sha3_512(path)` | SHA3-512 |
| `luapilot.blake2s(path)` | BLAKE2s-256 |
| `luapilot.blake2b(path)` | BLAKE2b-512 |

Each returns `string` (hex) or `(nil, err)`. MD5 and SHA-1 are
exposed for compatibility (Git, legacy systems) — don't use them
for new cryptographic uses.

## Quick examples

```lua
-- Predicates
if luapilot.isdir("/etc") and luapilot.fileExists("/etc/hostname") then
    print("size:", luapilot.fileSize("/etc/hostname"))
end

-- Recursive listing
for entry in luapilot.find("/var/log", { pattern = "*.log", type = "f", recursive = true }) do
    print(entry)
end

-- Copy with attributes
luapilot.copyTree("./src", "/tmp/backup", { preserve = true, overwrite = true })

-- Hash a release tarball
local sha, err = luapilot.sha256("/tmp/luapilot-1.7.0.tar.gz")
if sha then print("SHA256:", sha) end
```

## Error contract

- All functions follow the `(nil, err)` convention on runtime
  failure (no such file, permission denied, etc.).
- Wrong argument **types** raise via `luaL_error` (passing a
  number where a path is expected after coercion has weird
  semantics).
- Path safety : `copy`, `copyTree`, `moveTree` use
  `lexically_relative()` and component-wise `is_within()` guards
  to prevent symlink-based escapes out of the destination tree.
  See [`security`](../security.md).

## Design decisions

- **Flat namespace** : `fileExists` instead of `fs.exists`. Pure
  historical reason — these functions predate the per-module
  convention. Stable now ; renaming would break every script.
- **Hashes operate on file paths**, not raw bytes. The common
  case is "hash this file", so the API takes a path. For raw
  bytes, the openssl-based implementation is internal — pull
  request welcome if a use case emerges.
- **`copy` is single-file, `copyTree` is recursive**. Lua doesn't
  have function overloading by type, so splitting them avoids the
  ambiguity "did `copy('dir', 'dir')` mean recursive or not ?".

## Not in v1

- Glob matching as a standalone function (`luapilot.glob`). Use
  `find` with `pattern = "*.lua"` for the common case.
- Async I/O. Use [`workers`](workers.md) for parallel file
  processing instead.
