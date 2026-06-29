> **English** | [Français](../../fr/modules/http.md)

# `babet.http` — HTTP client

A simple HTTP/HTTPS client built on
[cpp-httplib](https://github.com/yhirose/cpp-httplib), reusing the
vendored OpenSSL. Synchronous, blocking, with timeouts.

## Why

Almost every non-trivial script needs to talk to some HTTP API.
Without a built-in client, you reach for `curl` via `exec`, with
all the quoting and process-spawning overhead. `babet.http`
makes a request a one-liner, with proper TLS verification by
default.

## API

| Function | Returns |
| --- | --- |
| `babet.http.request(opts)` | `response` (table) \| `(nil, err)` |
| `babet.http.get(url, opts?)` | shortcut for `request{ method="GET", url=url, ... }` |
| `babet.http.post(url, opts?)` | shortcut for `request{ method="POST", url=url, ... }` |
| `babet.http.put(url, opts?)` | shortcut for `request{ method="PUT", url=url, ... }` |
| `babet.http.delete(url, opts?)` | shortcut for `request{ method="DELETE", url=url, ... }` |

### `opts` table

| Field | Type | Default |
| --- | --- | --- |
| `url` | string (required for `request`) | — |
| `method` | string | `"GET"` |
| `headers` | table of `name = value` | `{}` |
| `body` | string | `""` |
| `timeout` | number (seconds) | 30 |
| `verify` | boolean (TLS cert verification) | `true` |
| `ca_cert` | string (path to CA bundle file) | system default |
| `ca_path` | string (path to CA dir) | system default |
| `follow_redirects` | boolean | `true` |
| `max_redirects` | integer | 5 |

### `response` table

```lua
{
    status = 200,
    body = "...",
    headers = {                 -- normalised lowercase keys
        ["content-type"] = "application/json",
        ["content-length"] = "42",
    },
    -- final URL after redirects:
    url = "https://api.example.com/v1/data",
}
```

## Quick examples

```lua
-- Simple GET
local r, err = babet.http.get("https://api.example.com/health")
if r then
    print(r.status, r.body)
end

-- JSON POST with auth header
local r, err = babet.http.post("https://api.example.com/v1/things", {
    headers = {
        ["Content-Type"] = "application/json",
        ["Authorization"] = "Bearer " .. token,
    },
    body = babet.json.encode({ name = "widget", count = 7 }),
    timeout = 10,
})

-- Self-signed cert (dev / private CA)
local r = babet.http.get("https://internal.svc/", {
    ca_cert = "/etc/myapp/internal-ca.crt",
})

-- Disable verification (TESTING ONLY)
local r = babet.http.get("https://expired.badssl.com/",
                            { verify = false })
```

## Error contract

- **Wrong argument types** → raises via `luaL_error`.
- **Network errors** (DNS, connect, timeout, TLS) →
  `(nil, "http: <description>")`.
- **HTTP status errors** are **not** errors — a `404` returns a
  normal `response` table with `status = 404`. Status semantics
  are the caller's responsibility.

## TLS / trust store

Babet ships its own static OpenSSL, so it doesn't automatically
inherit the distro's `ca-certificates` configuration. Two
mechanisms ensure `verify=true` works out of the box :

1. The vendored OpenSSL is built with `--openssldir=/etc/ssl`,
   covering Arch, Debian, Ubuntu, Alpine, Gentoo.
2. At TLS init, Babet probes known CA bundle paths for
   Fedora/RHEL, OpenSUSE, FreeBSD, NetBSD.

You can override per-call with `ca_cert` / `ca_path`, or globally
via `SSL_CERT_FILE` / `SSL_CERT_DIR` environment variables. See
[`security`](../security.md) and [`tls`](tls.md) for details.

## Design decisions

- **Synchronous, blocking**. Scripts are usually one-shot
  request-response affairs ; sync is simpler and sufficient. For
  many parallel calls, use [`workers`](workers.md).
- **`verify=true` by default**. Disabling TLS verification must
  be explicit (`verify=false`). No silent downgrade.
- **HTTP status is not an error**. The caller decides whether
  `404` or `500` is failure ; the network call itself succeeded.
- **Headers are normalised to lowercase keys** in the response,
  to make case-insensitive lookups work directly
  (`r.headers["content-type"]`).
- **Follow redirects by default**. Bounded to 5 hops, override
  with `max_redirects` or disable with `follow_redirects=false`.

## Not in v1

- Streaming response body (e.g. for downloading large files).
  Currently `body` is read fully into memory.
- HTTP/2 / HTTP/3.
- WebSockets (use [`socket`](socket.md) + TLS + a Lua framing
  library if needed).
- Server side. cpp-httplib supports it, but exposing a robust HTTP
  server to Lua is its own design effort.
