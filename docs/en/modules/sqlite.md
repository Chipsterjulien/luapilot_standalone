> **English** | [Français](../../fr/modules/sqlite.md)

# `luapilot.sqlite` — embedded SQL database

Wraps SQLite 3.53.1 (vendored), the most-deployed database engine
in the world. Zero-config persistence for any script that needs
more than a JSON file but less than a database server.

## Why

A script that needs to keep state across runs (a bot's seen-list,
a scraper's progress, accumulated metrics) shouldn't have to pull
in PostgreSQL or invent its own file format. SQLite is exactly
right at this scale : single file, transactional, fast, no daemon.

The Lua API mirrors the SQLite C API closely (open / exec /
prepare / step / finalize) with safer defaults and Lua-friendly
error returns.

## API

### Opening and closing

| Function | Returns |
| --- | --- |
| `luapilot.sqlite.open(path, opts?)` | `db` (userdata) \| `(nil, err)` |
| `db:close()` | `(true, nil)` — idempotent |

`opts` :

| Field | Type | Default |
| --- | --- | --- |
| `readonly` | boolean | `false` |
| `wal` | boolean — enable WAL journal mode | `false` |
| `busy_timeout` | number ms (block this long on a locked db) | `5000` |
| `foreign_keys` | boolean | `true` |

Special path `":memory:"` opens an in-memory database (lost on
close). Use for tests or transient processing.

### Direct exec (no parameters / one-shot statement)

| Function | Returns |
| --- | --- |
| `db:exec(sql)` | `(true, nil)` \| `(nil, err)` |
| `db:exec(sql, params)` | same, with `?`-bound parameters |
| `db:query(sql, params?)` | `table` of row tables \| `(nil, err)` |

### Prepared statements (re-use, performance)

| Function | Returns |
| --- | --- |
| `db:prepare(sql)` | `stmt` (userdata) \| `(nil, err)` |
| `stmt(params?)` | iterator over rows |
| `stmt:exec(params?)` | `(true, nil)` \| `(nil, err)` |
| `stmt:finalize()` | `(true, nil)` — idempotent |

### Transactions

| Function | Returns |
| --- | --- |
| `db:transaction(fn)` | `(true, result)` if `fn` returns truthy ; rollback + `(false, err)` otherwise |

### Utilities

| Function | Returns |
| --- | --- |
| `db:last_insert_rowid()` | `integer` |
| `db:changes()` | `integer` — rows affected by last write |
| `db:in_transaction()` | `boolean` |

## Type mapping

| SQLite type | Lua type |
| --- | --- |
| INTEGER | integer |
| REAL | number (float) |
| TEXT | string |
| BLOB | string (binary-safe) |
| NULL | `nil` (read), `nil` or `db.NULL` sentinel (write) |

## Quick examples

```lua
local db = assert(luapilot.sqlite.open("state.db", { wal = true }))

-- Schema
db:exec([[
    CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT UNIQUE NOT NULL,
        active INTEGER DEFAULT 1
    );
]])

-- Insert with parameters
assert(db:exec("INSERT INTO users (name) VALUES (?)", { "alice" }))
print("inserted as id", db:last_insert_rowid())

-- Query rows
local rows = assert(db:query("SELECT id, name FROM users WHERE active = ?", { 1 }))
for _, r in ipairs(rows) do
    print(r.id, r.name)
end

-- Prepared statement reused in a loop (fast path)
local stmt = assert(db:prepare("INSERT INTO users (name) VALUES (?)"))
for _, name in ipairs({ "bob", "carol", "dave" }) do
    assert(stmt:exec({ name }))
end
stmt:finalize()

-- Transaction
local ok, err = db:transaction(function()
    db:exec("UPDATE users SET active = 0 WHERE name = ?", { "alice" })
    db:exec("UPDATE users SET active = 1 WHERE name = ?", { "bob" })
    return true   -- commit
end)
if not ok then print("rollback:", err) end

db:close()
```

## Error contract

- All runtime errors → `(nil, "sqlite: <description>")` with the
  SQLite error code in the message.
- **`exec(sql, params)` with `?` placeholders but no `params`
  argument** → `(nil, "sqlite: placeholders found but no params
  table provided")` — explicit error rather than silent NULL
  binding, which is a classic injection footgun.
- **Sparse keys in `params`** (e.g. `{[1] = "a", [3] = "c"}`) →
  `(nil, err)`.
- **Empty BLOB string** → stored correctly as empty BLOB
  (previously buggy in pre-1.5).
- **Methods after `close`** → `(nil, "sqlite: db closed")`.
- **Wrong argument types** → raises via `luaL_error`.

## Design decisions

- **WAL is opt-in, not default**. WAL is strictly better for most
  use cases, but it creates `*-wal` and `*-shm` sidecar files
  that some users find surprising. Opt-in keeps the default
  surprise-free, but every long-running daemon should pass
  `wal=true`.
- **Foreign keys ON by default**, contrary to SQLite default.
  Most modern schemas rely on FK constraints ; defaulting them
  off is a footgun.
- **`transaction(fn)` commits if `fn` returns truthy**.
  Conventional in Lua : return a value to signal success, return
  nothing/`nil`/`false` (or raise) to rollback. The function's
  return value is propagated.
- **Placeholders without `params` is an error**, not a silent
  NULL bind. Catches a class of injection-adjacent bugs early.
- **Errors are returned, not raised**. Even malformed SQL returns
  `(nil, err)`. Scripts that don't check are noisy but not
  catastrophic.

## Not in v1

- `BLOB` streaming I/O (`sqlite3_blob_open`). Use string
  serialisation if you fit in memory.
- Virtual tables / FTS5 / R-Tree. Available via raw SQL if the
  vendored SQLite is built with them ; no Lua-level API yet.
- Backup API (`sqlite3_backup_init`). For now, `VACUUM INTO
  'backup.db'` works as a one-liner.

For a higher-level ORM-like layer, build it in Lua on top of
this API.
