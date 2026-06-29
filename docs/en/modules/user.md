> **English** | [Français](../../fr/modules/user.md)

# `babet.user`

System user lookups via NSS, without parsing `/etc/passwd` directly.

## Why

On a modern Linux system, users may come from multiple sources :
`/etc/passwd`, LDAP, SSSD, NIS+, FreeIPA, systemd-userdb, or other
NSS plugins. Reading `/etc/passwd` directly misses everything except
the first source. `babet.user` is backed by `getpwnam_r(3)` and
`getpwuid_r(3)`, which traverse the resolver chain configured in
`/etc/nsswitch.conf` — exactly what `id` or `getent passwd` does.

## API

| Function | Returns |
| --- | --- |
| `user.get(name_or_uid)` | `table` \| `(nil, "user not found")` \| `(nil, "user: <err>")` |
| `user.exists(name_or_uid)` | `boolean` |

`name_or_uid` accepts :

- a **string** → looked up by name via `getpwnam_r`
- a **non-negative integer** → looked up by UID via `getpwuid_r`

Any other type, a float, a negative integer, a string containing an
embedded NUL byte, or an integer above `uid_t` max raise via
`luaL_error` — these are caller bugs, not runtime conditions to
handle gracefully.

### Returned table on success

```lua
{
    name  = "yaourt",
    uid   = 968,
    gid   = 968,
    gecos = "yaourt AUR build user",
    home  = "/var/cache/yaourt",
    shell = "/usr/sbin/nologin",
}
```

All string fields are guaranteed non-nil. They may be empty strings
when the underlying NSS source has no value (this is rare but
possible — typically empty `gecos`).

## Quick example

```lua
-- Ensure a service user is provisioned before launching the daemon.
if not babet.user.exists("yaourt") then
    error("yaourt user not provisioned — run useradd first")
end

local u = assert(babet.user.get("yaourt"))
babet.chdir(u.home)
```

## Error contract

- **Bad argument type** (`table`, `boolean`, `nil`, float, negative
  integer, UID above `uid_t` max, string with embedded NUL) → raises
  via `luaL_error`. Use `pcall` if you need to recover.
- **User absent** (the resolver returned no match) → `get()` returns
  `(nil, "user not found")` ; `exists()` returns `false`.
- **NSS error** (resolver itself broken : LDAP unreachable, out of
  memory, etc.) → `get()` returns `(nil, "user: <description>")` ;
  `exists()` returns `false` (errors are silently coerced into
  "absent" for prediates ; use `get()` if you need to diagnose).

## Design decisions

- **NSS only, never `/etc/passwd`** : reading the file directly would
  silently miss every account that isn't in the local file. The whole
  point of `babet.user` is that the script gets the same answer as
  `id` or `getent passwd`.
- **Thread-safe `_r` variants** : Babet exposes
  `babet.workers`, so we use `getpwnam_r`/`getpwuid_r` everywhere.
- **`gecos` exposed raw** : the historical format is
  `Full Name,Office,WorkPhone,HomePhone[,Other]`, but in practice
  most entries hold a free-form name. Parsing it in Lua is trivial
  if you need to ; baking a parser in C++ would be premature.
- **NUL byte rejection** : `"root\0evil"` would be seen as `"root"` by
  `getpwnam_r` (it stops at the first NUL). On user-derived input,
  that could bypass an upstream identity check. Explicit rejection
  is safer than implicit truncation.
- **`uid_t` range check** : on Linux, `uid_t` is `uint32_t`, while
  `lua_Integer` is `int64_t`. Without a check, `5_000_000_000` would
  be silently truncated to a 32-bit value and could match an
  unrelated account.

## Not in v1

Additive — these could be added later without breaking SemVer :

- Group lookups (`getgrnam` / `getgrgid`) — will be added when a
  concrete use case asks for them.
- `/etc/shadow` access (passwords, password aging) — out of scope.
  Requires root and is rarely useful outside niche admin scripts.
  Password authentication should go through PAM, not Babet.
- User/group creation — already feasible via
  `babet.exec("useradd …")` and `groupadd`. No need for a
  dedicated binding.
