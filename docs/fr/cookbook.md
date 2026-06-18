> [English](../en/cookbook.md) | **Français**

# Cookbook

Recettes concrètes pour des tâches de scripting courantes. Chaque
recette est courte et autonome ; copie, adapte, déploie.

## Surveiller un dossier et envoyer les nouveaux fichiers par email

Un dossier de dépôt qui mailise chaque fichier dès qu'il arrive.
Utilise `luapilot.inotify` pour le réveil instantané (pas de
polling), et écoute `close_write` + `moved_to` pour garantir que
le fichier est complet.

```lua
local w = assert(luapilot.inotify.new())
assert(w:add("/srv/incoming", { "close_write", "moved_to" }))

luapilot.signal.handle("TERM", function()
    w:close()
    os.exit(0)
end)

while true do
    local events, err = w:read()
    if not events then
        if err == "interrupted" then break end
        io.stderr:write("inotify: ", err, "\n"); break
    end
    for _, ev in ipairs(events) do
        if ev.events.overflow then
            -- queue saturée : rescanne le dossier
            -- (des événements ont été perdus)
        else
            local path = "/srv/incoming/" .. ev.name
            send_email(path)
            os.remove(path)
        end
    end
end
```

## S'assurer qu'un user système existe, puis basculer dessus

Check typique à l'install avant de lancer un daemon sous un
utilisateur non-root.

```lua
local function ensure_user(name)
    if luapilot.user.exists(name) then return true end
    local ok, err = luapilot.exec("useradd", {
        "--system",
        "--home-dir", "/var/lib/" .. name,
        "--create-home",
        "--shell", "/usr/sbin/nologin",
        name,
    })
    if not ok then
        return nil, "useradd a échoué : " .. tostring(err)
    end
    return true
end

assert(ensure_user("myapp"))
local u = assert(luapilot.user.get("myapp"))
luapilot.chdir(u.home)
```

## Récupérer beaucoup d'URLs en parallèle

`luapilot.workers` fait tourner du Lua sur plusieurs cœurs. Le temps
total est proche du fetch le plus lent, pas de la somme.

```lua
local urls = { "https://a.example/", "https://b.example/",
               "https://c.example/", "https://d.example/" }

local jobs = {}
for i, url in ipairs(urls) do
    jobs[i] = luapilot.workers.spawn([[
        local url = worker.args.url
        local r, err = luapilot.http.get(url, { timeout = 10 })
        if not r then return { error = err } end
        return { status = r.status, length = #r.body }
    ]], { url = url })
end

for i, job in ipairs(jobs) do
    local ok, result = job:join()
    print(i, ok, result and result.status or result.error)
end
```

## Arrêt propre d'un script long-running

Attraper `SIGTERM` / `SIGINT` pour que le script puisse fermer
ses ressources proprement. Fonctionne avec `systemctl stop` et
Ctrl-C.

```lua
local log = require("logging")
log.set_level("info")

local running = true
luapilot.signal.handle("TERM", function()
    log.info("SIGTERM, arrêt en cours")
    running = false
end)
luapilot.signal.handle("INT", function()
    log.info("Ctrl-C, arrêt en cours")
    running = false
end)

while running do
    -- boucle principale ; les appels bloquants renvoient
    -- (nil, "interrupted") quand un signal géré arrive, donc la
    -- boucle sort rapidement.
    local line, err = sock:recv_line()
    if not line then
        if err == "interrupted" then break end
        log.error("recv : ", err); break
    end
    process(line)
end

sock:close()
log.info("arrêt propre")
```

## Lire une config TOML, valider, persister en SQLite

```lua
-- Charger le TOML
local f = assert(io.open("config.toml", "r"))
local body = f:read("a"); f:close()
local cfg, perr = luapilot.toml.decode(body)
if not cfg then error("config.toml : " .. perr) end

-- Valider (checks manuels simples ; pour un vrai schéma, bâtis-le en Lua)
assert(type(cfg.server) == "table", "section [server] absente")
assert(type(cfg.server.host) == "string", "server.host absent")
assert(math.type(cfg.server.port) == "integer", "server.port doit être un entier")

-- Ouvrir la db, persister
local db = assert(luapilot.sqlite.open("state.db", { wal = true }))
db:exec([[CREATE TABLE IF NOT EXISTS config (k TEXT PRIMARY KEY, v TEXT)]])
db:exec("INSERT OR REPLACE INTO config VALUES (?, ?)",
        { "host", cfg.server.host })
db:exec("INSERT OR REPLACE INTO config VALUES (?, ?)",
        { "port", tostring(cfg.server.port) })
db:close()
```

## Cache avec TTL lisible, échéances en timestamps ISO

Parser un TTL depuis une config (`"15m"`, `"2h30m"`, `"1d"`),
calculer une échéance, et la logger dans un format UTC uniforme
sur lequel `grep` à travers les fichiers de log marche vraiment.

```lua
-- Config depuis TOML, CLI, env, peu importe.
local raw_ttl = "15m"

local ttl, err = luapilot.time.parse_duration(raw_ttl)
if not ttl then error("TTL invalide '" .. raw_ttl .. "' : " .. err) end

local deadline = luapilot.now() + ttl
print(string.format("[%s] entrée cache créée, expire à %s (TTL %s)",
    luapilot.time.iso(),
    luapilot.time.iso(deadline),
    luapilot.time.format_duration(ttl)))

-- ...plus tard, ailleurs dans le script :
if luapilot.now() >= deadline then
    print("[" .. luapilot.time.iso() .. "] cache miss : entrée expirée")
end
```

Propriétés utiles de ce pattern :

- `luapilot.time.iso()` est toujours UTC, toujours 20 caractères,
  toujours la même forme. `grep` et `sort` marchent directement
  sur les timestamps.
- `parse_duration("15m")` renvoie un entier, donc `now() + ttl`
  est exact (pas de drift float sur les longues durées).
- `format_duration(ttl)` est aller-retour : `parse_duration(
  format_duration(n)) == n` pour tout `n >= 0`. Sûr à utiliser
  comme représentation canonique dans les logs ou les bases.

## Pretty-print d'une valeur Lua

Le module `inspect` (embarqué) donne une sortie lisible pour les
tables imbriquées.

```lua
local inspect = require("inspect")
print(inspect({ a = 1, b = { c = 2, d = { 3, 4, 5 } } }))
-- { a = 1, b = { c = 2, d = { 3, 4, 5 } } }
```

## Hello there

Le script LuaPilot minimum :

```lua
luapilot.helloThere()
```
